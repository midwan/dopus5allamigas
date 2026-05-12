#!/usr/bin/env python3
"""Static regression checks for Dual Lister plumbing."""

from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parents[3]
PROGRAM = ROOT / "source" / "Program"


def read_program_source(name):
    return (PROGRAM / name).read_text(encoding="latin-1")


def read_program_source_if_exists(name):
    path = PROGRAM / name
    if not path.exists():
        return ""
    return path.read_text(encoding="latin-1")


def read_repo_file(*parts):
    return (ROOT.joinpath(*parts)).read_text(encoding="latin-1")


class DualListerStaticTests(unittest.TestCase):
    def test_lister_menu_has_dual_lister_toggle(self):
        menu_data_h = read_program_source("menu_data.h")
        menu_data_c = read_program_source("menu_data.c")
        lister_menus_c = read_program_source("lister_menus.c")
        string_data_h = read_program_source("string_data.h")
        dopus_cd = read_program_source("dopus.cd")

        self.assertIn("MENU_LISTER_DUAL", menu_data_h)
        self.assertIn('MSG_LISTER_DUAL_STR "Dual Lister"', string_data_h)
        self.assertIn("MSG_LISTER_DUAL", dopus_cd)
        self.assertRegex(
            menu_data_c,
            r"MENU_LISTER_DUAL,\s*MSG_LISTER_DUAL,\s*CHECKIT\s*\|\s*MENUTOGGLE",
        )
        self.assertIn('{MENU_LISTER_DUAL, "Dual Lister"}', menu_data_c)
        self.assertIn("find_menu_item(menu, MENU_LISTER_DUAL)", lister_menus_c)

    def test_set_command_accepts_dual_keyword_and_toggle_values(self):
        function_set_c = read_program_source("function_set.c")

        self.assertIn("SET_DUAL", function_set_c)
        self.assertIn('"dual"', function_set_c)
        self.assertIn("LISTER_DUAL_TOGGLE", function_set_c)
        self.assertIn("LISTER_DUAL_ON", function_set_c)
        self.assertIn("LISTER_DUAL_OFF", function_set_c)
        self.assertRegex(
            function_set_c,
            r"IPC_Command\([^;]*LISTER_DUAL[^;]*\);",
            re.S,
        )

    def test_dual_lister_user_docs_are_updated(self):
        changelog = read_repo_file("ChangeLog")
        guide = read_repo_file("documents", "DOpus5.guide")

        self.assertIn("New Dual Lister mode", changelog)
        self.assertIn("Open New Listers in Dual Mode", changelog)
        self.assertIn('@node "Lister - Dual Lister" "Lister - Dual Lister"', guide)
        self.assertIn('link "Lister - Dual Lister"', guide)
        self.assertIn("Set Dual [On|Off|Toggle|0|1]", guide)
        self.assertIn("Open New Listers in Dual Mode:", guide)
        self.assertRegex(guide, r"The active panel is treated as the\s+source.*destination")

    def test_dual_lister_core_symbols_are_present(self):
        lister_h = read_program_source("lister.h")
        lister_dual_c = read_program_source_if_exists("lister_dual.c")
        lister_proc_c = read_program_source("lister_proc.c")
        lister_function_c = read_program_source("lister_function.c")
        lister_idcmp_c = read_program_source("lister_idcmp.c")
        makefile_objs = read_program_source("makefile.objs")

        self.assertIn("lister_dual.o", makefile_objs)
        self.assertRegex(lister_h, r"LISTER_DUAL,")
        self.assertRegex(lister_h, r"#define\s+LISTERF2_DUAL\s+\(1 << 1\)")
        self.assertNotIn("LISTERF2_DUAL_SYNC", lister_h)
        self.assertIn("char pad2[42];", lister_h)
        self.assertNotIn("struct ListerWindow *dual_partner;", lister_h)
        self.assertNotIn("struct ListerWindow *dual_owner;", lister_h)
        self.assertNotIn("lister_dual_get_partner", lister_dual_c)
        self.assertNotIn("lister_dual_get_owner", lister_dual_c)
        self.assertIn("state->flags |= DUALF_ONE_WINDOW;", lister_dual_c)
        self.assertIn("state->buffer[LISTER_DUAL_LEFT] = lister->cur_buffer;", lister_dual_c)
        self.assertIn("state->buffer[LISTER_DUAL_RIGHT] = lister->cur_buffer;", lister_dual_c)
        self.assertNotRegex(
            lister_dual_c,
            r"(?s)BOOL lister_dual_enter\(Lister \*lister\).*"
            r"lister_dual_position_partner\(partner, owner\);",
        )
        self.assertNotIn("lister_split_display(partner, owner)", lister_dual_c)
        self.assertIn("BOOL lister_dual_toggle(Lister *lister, short mode);", lister_h)
        self.assertIn("void lister_dual_activate(Lister *lister);", lister_h)
        self.assertNotIn("Lister *lister_dual_partner(Lister *lister);", lister_h)
        self.assertIn("BOOL lister_dual_is_side(Lister *lister);", lister_h)
        self.assertIn("if (lister->flags & LISTERF_VIEW_ICONS)", lister_dual_c)
        self.assertIn("case LISTER_DUAL:", lister_proc_c)
        self.assertIn("lister_dual_toggle(lister, flags);", lister_proc_c)
        self.assertIn("lister_dual_activate(lister);", lister_function_c)
        self.assertRegex(
            lister_idcmp_c,
            r"(?s)case IDCMP_ACTIVEWINDOW:.*lister_dual_activate\(lister\);",
        )

    def test_icon_mode_transition_leaves_dual_only_when_mode_change_can_proceed(self):
        lister_function_c = read_program_source("lister_function.c")
        icon_case = re.search(
            r"(?s)case MENU_LISTER_VIEW_ICON:.*?case MENU_LISTER_VIEW_NAME:",
            lister_function_c,
        ).group(0)

        before_window_check = icon_case.split("// Is lister not open?", 1)[0]
        self.assertNotIn("lister_dual_leave(lister);", before_window_check)
        self.assertRegex(
            icon_case,
            r"(?s)// Is lister not open\?.*?"
            r"if \(!lister->window\)\s*\{\s*"
            r"// Dual listers are Name-mode only\.\s*"
            r"if \(lister_dual_is_side\(lister\)\)\s*"
            r"lister_dual_leave\(lister\);.*?"
            r"// Set icon mode flag",
        )
        self.assertRegex(
            icon_case,
            r"(?s)// Can't change mode if busy\s*"
            r"if \(!\(lister->flags & LISTERF_BUSY\)\)\s*\{\s*"
            r"struct IBox dims, current;\s*"
            r"short extra;\s*"
            r"// Dual listers are Name-mode only\.\s*"
            r"if \(lister_dual_is_side\(lister\)\)\s*"
            r"lister_dual_leave\(lister\);\s*"
            r"// Get extra height",
        )

    def test_dual_lister_has_single_window_panel_model(self):
        lister_h = read_program_source("lister.h")
        lister_dual_c = read_program_source_if_exists("lister_dual.c")
        lister_display_c = read_program_source("lister_display.c")
        lister_show_c = read_program_source("lister_show.c")
        lister_idcmp_c = read_program_source("lister_idcmp.c")
        lister_proc_c = read_program_source("lister_proc.c")
        function_launch_c = read_program_source("function_launch.c")
        draw_panel = re.search(
            r"(?s)static void lister_dual_draw_panel\(Lister \*lister, short side\).*?"
            r"\n}\n\nBOOL lister_dual_update_pathfields",
            lister_dual_c,
        ).group(0)
        refresh_active_panel_match = re.search(
            r"(?s)static void lister_dual_refresh_active_panel\(Lister \*lister, ListerDualState \*state, ULONG flags\).*?"
            r"\n}\n\n// Refresh the dual display",
            lister_dual_c,
        )
        self.assertIsNotNone(refresh_active_panel_match)
        refresh_active_panel = refresh_active_panel_match.group(0)
        dual_refresh = re.search(
            r"(?s)void lister_dual_refresh\(Lister \*lister, unsigned short mode\).*?"
            r"\n}\n\nstatic void lister_dual_refresh_active_panel",
            lister_dual_c,
        ).group(0)
        dual_refresh_display = re.search(
            r"(?s)void lister_dual_refresh_display\(Lister \*lister, ULONG flags\).*?"
            r"\n}\n\nvoid lister_dual_close_display",
            lister_dual_c,
        ).group(0)
        create_scrollbars_func = re.search(
            r"(?s)static BOOL lister_dual_create_scrollbar_side\(Lister \*lister, ListerDualState \*state, short side\).*?"
            r"\n}\n\nstatic void lister_dual_remove_scrollbars",
            lister_dual_c,
        ).group(0)
        layout_func = re.search(
            r"(?s)static void lister_dual_layout\(Lister \*lister, ListerDualState \*state\).*?"
            r"\n}\n\nstatic void lister_dual_begin_panel_clip",
            lister_dual_c,
        ).group(0)

        self.assertIn("BOOL lister_dual_one_window(Lister *lister);", lister_h)
        self.assertIn("BOOL lister_dual_init_display(Lister *lister);", lister_h)
        self.assertIn("void lister_dual_refresh(Lister *lister, unsigned short mode);", lister_h)
        self.assertIn("void lister_dual_refresh_display(Lister *lister, ULONG flags);", lister_h)
        self.assertIn("void lister_dual_activate_at(Lister *lister, UWORD x, UWORD y);", lister_h)
        self.assertIn("char *lister_dual_dest_path(Lister *lister);", lister_h)
        self.assertIn("BOOL lister_dual_update_pathfields(Lister *lister);", lister_h)
        self.assertIn("BOOL lister_dual_disable_pathfields(Lister *lister, short disable);", lister_h)
        self.assertIn("BOOL lister_dual_handle_path_gadget(Lister *lister, struct Gadget *gadget);", lister_h)
        self.assertIn("BOOL lister_dual_activate_pathfield(Lister *lister);", lister_h)
        self.assertIn("BOOL lister_dual_update_sliders(Lister *lister, int which);", lister_h)
        self.assertIn("BOOL lister_dual_handle_scroll_gadget(Lister *lister, struct Gadget *gadget, short which);", lister_h)
        self.assertIn("BOOL lister_dual_handle_close_button(Lister *lister, UWORD x, UWORD y);", lister_h)
        self.assertIn("BOOL lister_dual_help_at(Lister *lister, UWORD x, UWORD y);", lister_h)
        self.assertNotIn("BOOL lister_dual_suppress_toolbar(Lister *lister);", lister_h)
        self.assertNotIn("void lister_dual_sync_partner(Lister *lister);", lister_h)

        self.assertIn("typedef struct ListerDualState", lister_dual_c)
        self.assertIn("sizeof(((Lister *)0)->pad2) >= sizeof(APTR)", lister_dual_c)
        self.assertIn("DUALF_ONE_WINDOW", lister_dual_c)
        self.assertIn("DirBuffer *buffer[LISTER_DUAL_SIDES];", lister_dual_c)
        self.assertIn("DirBuffer *offset_buffer[LISTER_DUAL_SIDES];", lister_dual_c)
        self.assertIn("long vert_offset[LISTER_DUAL_SIDES];", lister_dual_c)
        self.assertIn("long horiz_offset[LISTER_DUAL_SIDES];", lister_dual_c)
        self.assertIn("GUI_Element list_area[LISTER_DUAL_SIDES];", lister_dual_c)
        self.assertIn("GUI_Element text_area[LISTER_DUAL_SIDES];", lister_dual_c)
        self.assertIn("GUI_Element close_area[LISTER_DUAL_SIDES];", lister_dual_c)
        self.assertNotIn("GUI_Element status_area[LISTER_DUAL_SIDES];", lister_dual_c)
        self.assertIn("struct Rectangle header_rect[LISTER_DUAL_SIDES];", lister_dual_c)
        self.assertIn("struct Gadget *path_field[LISTER_DUAL_SIDES];", lister_dual_c)
        self.assertIn("char path_buffer[LISTER_DUAL_SIDES][512];", lister_dual_c)
        self.assertIn("struct Gadget *vert_scroller[LISTER_DUAL_SIDES];", lister_dual_c)
        self.assertIn("struct Gadget *horiz_scroller[LISTER_DUAL_SIDES];", lister_dual_c)
        self.assertIn("struct Rectangle separator;", lister_dual_c)
        self.assertIn("DUALF_PATHFIELD_OWNED", lister_dual_c)
        self.assertIn("static BOOL lister_dual_add_pathfield(Lister *lister, ListerDualState *state)", lister_dual_c)
        self.assertIn("static void lister_dual_apply_panel(Lister *lister, short side)", lister_dual_c)
        self.assertIn("static void lister_dual_init_side_offsets(ListerDualState *state, short side)", lister_dual_c)
        self.assertIn("static void lister_dual_apply_side_offsets(ListerDualState *state, short side)", lister_dual_c)
        self.assertIn("static void lister_dual_capture_side_offsets(Lister *lister, ListerDualState *state, short side)", lister_dual_c)
        self.assertIn("static void lister_dual_capture_active_buffer(Lister *lister)", lister_dual_c)
        self.assertIn("static BOOL lister_dual_ensure_pathfields(Lister *lister, ListerDualState *state)", lister_dual_c)
        self.assertIn("static BOOL lister_dual_ensure_scrollbars(Lister *lister, ListerDualState *state)", lister_dual_c)
        self.assertIn("static void lister_dual_remove_scrollbars(Lister *lister, ListerDualState *state)", lister_dual_c)
        self.assertIn("static short lister_dual_bottom_scroll_height(Lister *lister)", lister_dual_c)
        self.assertIn("static void lister_dual_prepare_resize_refresh(Lister *lister)", lister_dual_c)
        self.assertIn("static void lister_dual_draw_close_button(Lister *lister, ListerDualState *state, short side)", lister_dual_c)
        self.assertIn("static void lister_dual_draw_header(Lister *lister, ListerDualState *state, short side)", lister_dual_c)
        self.assertIn("static void lister_dual_close_side(Lister *lister, short side)", lister_dual_c)
        self.assertIn("static void lister_dual_clear_toolbar_area(Lister *lister)", lister_dual_c)
        self.assertIn("static void lister_dual_begin_panel_clip(Lister *lister, ListerDualState *state, short side)", lister_dual_c)
        self.assertIn("static void lister_dual_end_panel_clip(Lister *lister)", lister_dual_c)
        self.assertIn("static void lister_dual_draw_separator(Lister *lister, ListerDualState *state)", lister_dual_c)
        self.assertIn("static void lister_dual_draw_panel(Lister *lister, short side)", lister_dual_c)
        self.assertNotIn("lister_show_status(lister);", draw_panel)
        self.assertNotIn("display_gui_border(&lister->status_area);", lister_dual_c)
        self.assertNotIn("MSG_LISTER_STATUS_SOURCE", lister_dual_c)
        self.assertNotIn("MSG_LISTER_STATUS_DEST", lister_dual_c)
        self.assertRegex(
            lister_dual_c,
            r"(?s)state->header_rect\[panel\].MinX = panel_left\[panel\];.*"
            r"state->header_rect\[panel\].MaxX = panel_left\[panel\] \+ content_width - 1;",
        )
        self.assertIn(
            "state->close_area[panel].box.Left = panel_left[panel] + content_width;",
            lister_dual_c,
        )
        self.assertIn("state->close_area[panel].box.Width = scroll_width;", lister_dual_c)
        self.assertIn("state->close_area[panel].box.Top = state->header_rect[panel].MinY;", lister_dual_c)
        self.assertIn("static void lister_dual_clear_top_gutter(Lister *lister, ListerDualState *state, short side)", lister_dual_c)
        self.assertIn("static void lister_dual_clear_gadget_box(Lister *lister, struct Gadget *gadget)", lister_dual_c)
        self.assertIn("static void lister_dual_clear_gadget_chain(Lister *lister, struct Gadget *gadget, short count)", lister_dual_c)
        self.assertRegex(
            lister_dual_c,
            r"(?s)static BOOL lister_dual_gadget_rect.*"
            r"GFLG_RELRIGHT.*GFLG_RELBOTTOM.*GFLG_RELWIDTH.*GFLG_RELHEIGHT",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)static void lister_dual_clear_gadget_box.*"
            r"gadget->Flags & GFLG_RELBOTTOM.*"
            r"FILLPEN.*"
            r"BACKGROUNDPEN",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)static void lister_dual_draw_header.*"
            r"lister_dual_clear_top_gutter\(lister, state, side\);.*"
            r"state->list_area\[side\].rect.MinX.*state->header_rect\[side\].MinX - 1.*"
            r"RectFill\(&lister->name_area.rast,.*state->header_rect\[side\].MinX.*"
            r"lister_show_name\(lister\);.*"
            r"select_show_info\(lister, 0\);.*"
            r"do3dbox\(&lister->name_area.rast, &state->header_rect\[side\], 0\);.*"
            r"lister_dual_draw_close_button\(lister, state, side\);",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)static void lister_dual_position_scrollbars.*"
            r"lister_dual_clear_gadget_box\(lister, state->vert_scroller\[side\]\);.*"
            r"lister_dual_clear_gadget_box\(lister, state->horiz_arrow_right\[side\]\);",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)static BOOL lister_dual_add_pathfield.*"
            r"AddGList\(lister->window, lister->path_field, -1, 1, 0\);.*"
            r"state->flags \|= DUALF_PATHFIELD_OWNED;",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)static BOOL lister_dual_ensure_pathfields.*"
            r"RemoveGList\(lister->window, &lister->parent_button, 2\);.*"
            r"RefreshWindowFrame\(lister->window\);.*"
            r"AddGList\(lister->window, lister->path_field, -1, 1, 0\);",
        )
        ensure_pathfields = re.search(
            r"(?s)\nstatic BOOL lister_dual_ensure_pathfields\(Lister \*lister, ListerDualState \*state\)\n\{.*?"
            r"\n}\n\nstatic void lister_dual_refresh_pathfields",
            lister_dual_c,
        ).group(0)
        self.assertNotIn("lister_dual_clear_gadget_box(lister, &lister->parent_button);", ensure_pathfields)
        self.assertIn("lister_dual_restore_pathfield(lister, state);", ensure_pathfields)
        self.assertRegex(
            lister_dual_c,
            r"(?s)static void lister_dual_restore_pathfield.*"
            r"RemoveGList\(lister->window, lister->path_field, 1\);.*"
            r"AddGList\(lister->window, &lister->parent_button, -1, 2, 0\);.*"
            r"RefreshWindowFrame\(lister->window\);",
        )
        capture_active_buffer = re.search(
            r"(?s)static void lister_dual_capture_active_buffer\(Lister \*lister\).*?\n}\n\nstatic short",
            lister_dual_c,
        ).group(0)
        self.assertNotIn("state->buffer[other] == lister->old_buffer", capture_active_buffer)
        self.assertRegex(
            capture_active_buffer,
            r"(?s)lister->cur_buffer == lister->special_buffer.*"
            r"other = lister_dual_other_side\(state->active\);.*"
            r"!state->buffer\[other\].*"
            r"!state->buffer\[other\]->buf_Path\[0\].*"
            r"state->buffer\[other\] = lister->cur_buffer;",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)static short lister_dual_bottom_scroll_height.*"
            r"height = lister->window->BorderBottom - 4;",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)static void lister_dual_scroll_rects.*"
            r"bottom_strip_height = lister_dual_bottom_scroll_height\(lister\);.*"
            r"left_arrow_w = lister->window->BorderRight;.*"
            r"right_arrow_w = lister->window->BorderRight;.*"
            r"horiz_arrow_height = bottom_strip_height;.*"
            r"left_arrow_w = \(\(struct Image \*\)state->scroll_image\[side\]\[0\]\)->Width;.*"
            r"right_arrow_w = \(\(struct Image \*\)state->scroll_image\[side\]\[2\]\)->Width;.*"
            r"if \(bar_w < left_arrow_w\).*"
            r"bar_w = left_arrow_w;.*"
            r"if \(bar_w < right_arrow_w\).*"
            r"bar_w = right_arrow_w;.*"
            r"bottom_strip_top = lister->window->Height - lister->window->BorderBottom \+ 3;.*"
            r"bottom_strip_bottom = bottom_strip_top \+ bottom_strip_height - 1;.*"
            r"right->MaxX = panel_right;.*"
            r"right->MinX = right->MaxX - right_arrow_w \+ 1;.*"
            r"horiz_arrow_top = lister->window->Height - lister->window->BorderBottom \+ 1;.*"
            r"horiz_arrow_height = lister->window->BorderBottom;.*"
            r"right->MinY = horiz_arrow_top;.*"
            r"right->MaxY = horiz_arrow_top \+ horiz_arrow_height - 1;.*"
            r"left->MaxX = right->MinX - 1;.*"
            r"left->MinX = left->MaxX - left_arrow_w \+ 1;.*"
            r"left->MinY = right->MinY;.*"
            r"left->MaxY = right->MaxY;.*"
            r"horiz->MinX = panel_left;.*"
            r"horiz->MaxX = left->MinX - 4;.*"
            r"horiz->MinY = bottom_top;.*"
            r"horiz->MaxY = bottom_top \+ bar_h - 1;",
        )
        self.assertNotIn("horiz_arrow_top = lister->window->Height - lister->window->BorderBottom;", lister_dual_c)
        self.assertNotIn("horiz_arrow_height = lister->window->BorderBottom - 1", lister_dual_c)
        self.assertNotIn("horiz->MinY = right->MinY + 1", lister_dual_c)
        self.assertNotIn("horiz->MaxY = right->MaxY - 1", lister_dual_c)
        self.assertNotIn("bottom_strip_bottom = lister->window->Height - 2", lister_dual_c)
        self.assertNotIn("bottom_strip_top = lister->window->Height - lister->window->BorderBottom;", lister_dual_c)
        self.assertNotIn("left->MinX = panel_left", lister_dual_c)
        self.assertNotIn("right->MaxX = panel_right - bar_w", lister_dual_c)
        self.assertNotIn("horiz->MaxX = left->MinX - 1", lister_dual_c)
        self.assertNotIn("bottom_strip_bottom = lister->window->Height - 1", lister_dual_c)
        self.assertNotIn("horiz_arrow_height = ((struct Image *)state->scroll_image[side][0])->Height", lister_dual_c)
        self.assertNotIn("horiz_arrow_height = ((struct Image *)state->scroll_image[side][2])->Height", lister_dual_c)
        self.assertNotIn("left_arrow_w = 8", lister_dual_c)
        self.assertNotIn("right_arrow_w = 8", lister_dual_c)
        self.assertRegex(
            layout_func,
            r"(?s)path_bottom = lister->window->Height - lister->window->BorderBottom - 1;.*"
            r"lister->path_border.MaxY = path_bottom;",
        )
        self.assertNotIn("path_bottom = lister->window->Height - lister->window->BorderBottom - 2", layout_func)
        self.assertNotIn("bottom_scroll_height", layout_func)
        self.assertRegex(
            lister_dual_c,
            r"(?s)static void lister_dual_prepare_resize_refresh.*"
            r"lister->flags & LISTERF_RESIZED.*"
            r"rect.MinX = lister->window->BorderLeft;.*"
            r"rect.MinY = lister->window->BorderTop;.*"
            r"rect.MaxX = lister->window->Width - lister->window->BorderRight - 1;.*"
            r"rect.MaxY = lister->window->Height - lister->window->BorderBottom - 1;.*"
            r"EraseRect\(lister->window->RPort, rect.MinX, rect.MinY, rect.MaxX, rect.MaxY\);.*"
            r"OrRectRegion\(lister->refresh_extra, &rect\);.*"
            r"RefreshWindowFrame\(lister->window\);",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)static void lister_dual_resize_bottom_gadget.*"
            r"gadget->LeftEdge = rect->MinX;.*"
            r"gadget->Flags \|= GFLG_RELBOTTOM;.*"
            r"gadget->TopEdge = rect->MinY - lister->window->Height;.*"
            r"gadget->Width = lister_dual_rect_width\(rect\);.*"
            r"gadget->Height = lister_dual_rect_height\(rect\);",
        )
        resize_bottom_func = re.search(
            r"(?s)static void lister_dual_resize_bottom_gadget\(Lister \*lister, struct Gadget \*gadget, struct Rectangle \*rect\).*?"
            r"\n}\n\nstatic void lister_dual_resize_path_gadget",
            lister_dual_c,
        ).group(0)
        self.assertNotIn("SetGadgetAttrs", resize_bottom_func)
        self.assertNotIn("gadget->TopEdge = rect->MinY;", resize_bottom_func)
        self.assertRegex(
            lister_dual_c,
            r"(?s)static void lister_dual_position_scrollbars.*"
            r"RemoveGList\(lister->window, state->vert_scroller\[side\], 6\);.*"
            r"lister_dual_resize_gadget\(lister, state->vert_scroller\[side\], &vert\);.*"
            r"lister_dual_resize_gadget\(lister, state->vert_arrow_up\[side\], &up\);.*"
            r"lister_dual_resize_gadget\(lister, state->vert_arrow_down\[side\], &down\);.*"
            r"lister_dual_resize_bottom_gadget\(lister, state->horiz_scroller\[side\], &horiz\);.*"
            r"lister_dual_resize_bottom_gadget\(lister, state->horiz_arrow_left\[side\], &left\);.*"
            r"lister_dual_resize_bottom_gadget\(lister, state->horiz_arrow_right\[side\], &right\);.*"
            r"AddGList\(lister->window, state->vert_scroller\[side\], pos, 6, 0\);",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)BOOL lister_dual_init_display.*"
            r"lister_dual_prepare_resize_refresh\(lister\);.*"
            r"lister_dual_layout\(lister, state\);",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)static BOOL lister_dual_create_scrollbar_side.*"
            r"if \(state->vert_scroller\[side\]\).*"
            r"RefreshGList\(state->vert_scroller\[side\], lister->window, 0, 6\);",
        )
        self.assertRegex(
            create_scrollbars_func,
            r"(?s)for \(a = 0; a < 4; a\+\+\).*"
            r"\"sysiclass\".*"
            r"SYSIA_Size.*"
            r"\(lister->window->WScreen->Flags & SCREENHIRES\) \? SYSISIZE_MEDRES"
            r".*SYSIA_Which.*LEFTIMAGE \+ a",
        )
        self.assertNotIn('"dopusiclass"', create_scrollbars_func)
        self.assertNotIn("DIA_Type", create_scrollbars_func)
        self.assertRegex(
            create_scrollbars_func,
            r"(?s)state->horiz_scroller\[side\].*"
            r"\"propgclass\".*"
            r"GA_BottomBorder.*"
            r"TRUE.*"
            r"state->horiz_arrow_left\[side\].*"
            r"\"buttongclass\".*"
            r"GA_BottomBorder.*"
            r"TRUE.*"
            r"state->horiz_arrow_right\[side\].*"
            r"\"buttongclass\".*"
            r"GA_BottomBorder.*"
            r"TRUE",
        )
        self.assertNotIn("vert_arrow_height", create_scrollbars_func)
        self.assertNotIn("IA_Height", create_scrollbars_func)
        self.assertIn("lister_dual_dispose_scrollbar_side(lister, state, side, FALSE);", create_scrollbars_func)
        self.assertRegex(
            lister_dual_c,
            r"(?s)static BOOL lister_dual_scroll_metrics_changed.*"
            r"state->scroll_screen_flags.*"
            r"lister->window->WScreen->Flags.*SCREENHIRES.*"
            r"state->scroll_border_right.*lister->window->BorderRight.*"
            r"state->scroll_border_bottom.*lister->window->BorderBottom",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)static BOOL lister_dual_ensure_scrollbars.*"
            r"lister_dual_scroll_metrics_changed\(lister, state\).*"
            r"lister_dual_dispose_dual_scrollbars\(lister, state\);",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)static BOOL lister_dual_ensure_scrollbars.*"
            r"state->system_scroller = lister->backdrop_info->vert_scroller;.*"
            r"RemoveGList\(lister->window, state->system_scroller, 6\);.*"
            r"lister_dual_clear_gadget_chain\(lister, state->system_scroller, 6\);.*"
            r"RefreshWindowFrame\(lister->window\);",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)static void lister_dual_remove_scrollbars.*"
            r"AddGList\(lister->window, state->system_scroller, -1, 6, 0\);.*"
            r"RefreshGList\(state->system_scroller, lister->window, 0, 6\);.*"
            r"RefreshWindowFrame\(lister->window\);",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)static void lister_dual_draw_panel.*"
            r"lister_dual_apply_panel\(lister, side\);.*"
            r"lister_dual_draw_header\(lister, state, side\);",
        )
        self.assertIn("state->title_area[panel].box.Width = content_width;", lister_dual_c)
        self.assertRegex(
            lister_dual_c,
            r"(?s)static void lister_dual_begin_panel_clip.*"
            r"ClearRegion\(lister->title_region\).*"
            r"OrRectRegion\(lister->title_region, &clip\).*"
            r"InstallClipRegion\(lister->window->WLayer, lister->title_region\);",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)static void lister_dual_draw_panel.*"
            r"lister_init_lister_area\(lister\);.*"
            r"lister_dual_begin_panel_clip\(lister, state, side\);.*"
            r"lister_display_dir\(lister\);.*"
            r"lister_dual_capture_side_offsets\(lister, state, side\);.*"
            r"lister_dual_end_panel_clip\(lister\);",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)void lister_dual_refresh.*"
            r"lister_dual_draw_panel\(lister, side\);.*"
            r"lister_dual_draw_separator\(lister, state\);",
        )
        self.assertRegex(
            dual_refresh,
            r"(?s)if \(mode & LREFRESH_REFRESH\).*BeginRefresh\(lister->window\);.*"
            r"if \(mode & LREFRESH_REFRESH\).*EndRefresh\(lister->window, TRUE\);",
        )
        self.assertNotIn("LREFRESH_REFRESH | LREFRESH_SIMPLEREFRESH", dual_refresh)
        self.assertRegex(
            refresh_active_panel,
            r"(?s)side = state->active;.*"
            r"lister_dual_apply_panel\(lister, side\);.*"
            r"if \(flags & REFRESHF_RESORT\).*"
            r"lister_resort\(lister, \(flags & REFRESHF_SORTSEL\) \? 1 : 0\);.*"
            r"if \(flags & REFRESHF_UPDATE_NAME\).*"
            r"lister_refresh_name\(lister\);.*"
            r"else if \(flags & REFRESHF_NAME\).*"
            r"lister_show_name\(lister\);.*"
            r"lister_dual_draw_panel\(lister, side\);.*"
            r"lister_dual_apply_panel\(lister, side\);",
        )
        self.assertNotRegex(refresh_active_panel, r"for \(side = 0; side < LISTER_DUAL_SIDES; side\+\+\)")
        self.assertRegex(
            dual_refresh_display,
            r"(?s)if \(flags & \(REFRESHF_FULL \| REFRESHF_REFRESH \| REFRESHF_COLOUR\)\).*"
            r"lister_dual_refresh\(lister, \(flags & REFRESHF_REFRESH\) \? LREFRESH_REFRESH : LREFRESH_FULL\);.*"
            r"return;.*"
            r"lister_dual_refresh_active_panel\(lister, state, flags\);",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)BOOL lister_dual_update_pathfields.*"
            r"SetGadgetAttrs\(state->path_field\[side\].*"
            r"state->buffer\[side\]->buf_ExpandedPath",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)BOOL lister_dual_activate_pathfield.*"
            r"state->path_field\[LISTER_DUAL_LEFT\] = lister->path_field;.*"
            r"gadget = state->path_field\[state->active\];.*"
            r"lister_dual_apply_panel\(lister, state->active\);.*"
            r"ActivateStrGad\(gadget, lister->window\);",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)BOOL lister_dual_update_sliders.*"
            r"lister_dual_capture_side_offsets\(lister, state, state->active\);.*"
            r"PGA_Top,\s*state->vert_offset\[side\].*"
            r"PGA_Top,\s*state->horiz_offset\[side\]",
        )
        scroll_handler = re.search(
            r"(?s)BOOL lister_dual_handle_scroll_gadget\(Lister \*lister, struct Gadget \*gadget, short which\).*?"
            r"\n}\n\n// Is this lister part",
            lister_dual_c,
        ).group(0)
        self.assertIn("old_active = state->active;", scroll_handler)
        self.assertIn("lister_dual_capture_side_offsets(lister, state, side);", scroll_handler)
        self.assertRegex(
            scroll_handler,
            r"(?s)if \(old_active != side\).*"
            r"lister_dual_apply_panel\(lister, LISTER_DUAL_LEFT\);.*"
            r"lister_dual_draw_header\(lister, state, LISTER_DUAL_LEFT\);.*"
            r"lister_dual_apply_panel\(lister, LISTER_DUAL_RIGHT\);.*"
            r"lister_dual_draw_header\(lister, state, LISTER_DUAL_RIGHT\);.*"
            r"lister_dual_apply_panel\(lister, side\);",
        )
        self.assertNotIn("lister_dual_refresh(lister, LREFRESH_FULL);", scroll_handler)
        self.assertNotRegex(
            lister_dual_c,
            r"(?s)BOOL lister_dual_enter\(Lister \*lister\).*"
            r"IPC_Command\(partner->ipc,\s*LISTER_OPEN",
        )

        self.assertRegex(
            lister_display_c,
            r"(?s)void lister_init_display\(Lister \*lister\).*"
            r"if \(lister_dual_init_display\(lister\)\)\s*return;",
        )
        self.assertRegex(
            lister_display_c,
            r"(?s)void lister_refresh\(Lister \*lister, unsigned short mode\).*"
            r"if \(lister_dual_one_window\(lister\)\).*"
            r"lister_dual_refresh\(lister, mode\);",
        )
        self.assertRegex(
            lister_show_c,
            r"(?s)void lister_refresh_display\(Lister \*lister, ULONG flags\).*"
            r"if \(lister_dual_one_window\(lister\)\).*"
            r"lister_dual_refresh_display\(lister, flags\);",
        )
        self.assertRegex(
            lister_idcmp_c,
            r"(?s)case IDCMP_MOUSEBUTTONS:.*"
            r"if \(msg->Code == SELECTDOWN && "
            r"lister_dual_handle_close_button\(lister, msg->MouseX, msg->MouseY\)\)\s*"
            r"break;.*"
            r"lister_dual_activate_at\(lister, msg->MouseX, msg->MouseY\);",
        )
        self.assertRegex(
            lister_idcmp_c,
            r"(?s)case IDCMP_MOUSEBUTTONS:.*"
            r"if \(msg->Qualifier != 0xffff && lister_dual_is_side\(lister\) && "
            r"!lister_dual_one_window\(lister\) &&\s*"
            r"!lister_dual_popup_at\(lister, msg->MouseX, msg->MouseY\)\)\s*"
            r"lister_dual_to_front\(lister\);",
        )
        self.assertRegex(
            lister_idcmp_c,
            r"(?s)case GAD_VERT_SCROLLER:.*"
            r"if \(lister_dual_handle_scroll_gadget\(lister, gadget, SLIDER_VERT\)\).*"
            r"case GAD_HORIZ_SCROLLER:.*"
            r"if \(lister_dual_handle_scroll_gadget\(lister, gadget, SLIDER_HORZ\)\)",
        )
        self.assertRegex(
            lister_idcmp_c,
            r"(?s)case GAD_PATH:.*"
            r"if \(lister_dual_handle_path_gadget\(lister, gadget\)\)",
        )
        self.assertRegex(
            lister_idcmp_c,
            r"(?s)// Activate path field.*"
            r"if \(key_ok && !lister_dual_activate_pathfield\(lister\)\)\s*"
            r"ActivateStrGad\(lister->path_field, lister->window\);",
        )
        self.assertRegex(
            lister_display_c,
            r"(?s)void lister_update_pathfield\(Lister \*lister\).*"
            r"if \(lister_dual_update_pathfields\(lister\)\)\s*return;",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)void lister_dual_activate_at\(Lister \*lister, UWORD x, UWORD y\).*"
            r"point_in_element\(&state->name_area\[side\], x, y\).*"
            r"old_active != side.*"
            r"lister_dual_refresh\(lister, LREFRESH_FULL\);",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)BOOL lister_dual_handle_close_button\(Lister \*lister, UWORD x, UWORD y\).*"
            r"point_in_element\(&state->close_area\[side\], x, y\).*"
            r"lister_dual_close_side\(lister, side\);.*"
            r"return TRUE;",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)static void lister_dual_close_side\(Lister \*lister, short side\).*"
            r"keep_side = lister_dual_other_side\(side\);.*"
            r"state->active = keep_side;.*"
            r"lister_dual_leave\(lister\);",
        )
        self.assertRegex(
            lister_idcmp_c,
            r"(?s)// Or click on status display\?.*"
            r"msg->Code == SELECTDOWN && !lister_dual_one_window\(lister\).*"
            r"lister_listerpopup\(lister, msg->Code\);",
        )
        self.assertRegex(
            lister_idcmp_c,
            r"(?s)// Or on parent button\?.*"
            r"msg->Code == SELECTDOWN && !lister_dual_one_window\(lister\).*"
            r"lister_parent_popup\(lister, msg->Code\);",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)BOOL lister_dual_help_at\(Lister \*lister, UWORD x, UWORD y\).*"
            r"point_in_element\(&state->command_area\[side\], x, y\).*"
            r"help_show_help\(HELP_LISTER_MENU_POPUP, 0\);.*"
            r"point_in_element\(&state->name_area\[side\], x, y\).*"
            r"help_show_help\(HELP_LISTER_FILE_COUNT, 0\);",
        )
        self.assertRegex(
            lister_proc_c,
            r"(?s)// Is point over toolbar\?.*"
            r"else if \(lister_dual_help_at\(lister, x, y\)\).*"
            r"// Or over the menu popup\?",
        )
        self.assertIn("lister_dual_dest_path(source_list)", function_launch_c)
        self.assertNotIn("lister_dual_partner(source_list)", function_launch_c)

    def test_dual_lister_draws_titles_and_recalculates_each_panel(self):
        lister_dual_c = read_program_source_if_exists("lister_dual.c")
        draw_panel = re.search(
            r"(?s)static void lister_dual_draw_panel\(Lister \*lister, short side\).*?"
            r"\n}\n\nBOOL lister_dual_update_pathfields",
            lister_dual_c,
        ).group(0)

        self.assertIn("static void lister_dual_update_title_mode", lister_dual_c)
        self.assertRegex(
            lister_dual_c,
            r"(?s)BOOL lister_dual_init_display\(Lister \*lister\).*"
            r"lister_dual_update_title_mode\(lister, state\);.*"
            r"lister_dual_layout\(lister, state\);",
        )
        self.assertRegex(
            draw_panel,
            r"(?s)lister_dual_apply_panel\(lister, side\);.*"
            r"lister_dual_draw_header\(lister, state, side\);.*"
            r"lister->cur_buffer->buf_OldVertOffset = -1;.*"
            r"lister->cur_buffer->buf_OldHorizOffset = -1;.*"
            r"lister_fix_horiz_len\(lister\);.*"
            r"lister_display_dir\(lister\);",
        )

    def test_dual_lister_keeps_title_hit_boxes_per_panel(self):
        lister_dual_c = read_program_source_if_exists("lister_dual.c")
        apply_panel = re.search(
            r"(?s)static void lister_dual_apply_panel\(Lister \*lister, short side\).*?"
            r"\n}\n\nstatic void lister_dual_apply_active",
            lister_dual_c,
        ).group(0)
        draw_panel = re.search(
            r"(?s)static void lister_dual_draw_panel\(Lister \*lister, short side\).*?"
            r"\n}\n\nBOOL lister_dual_update_pathfields",
            lister_dual_c,
        ).group(0)

        self.assertIn("TitleBox title_boxes[LISTER_DUAL_SIDES][16];", lister_dual_c)
        self.assertIn("static void lister_dual_clear_title_boxes(TitleBox *boxes)", lister_dual_c)
        self.assertIn("static void lister_dual_capture_title_boxes", lister_dual_c)
        self.assertIn("static void lister_dual_apply_title_boxes", lister_dual_c)
        self.assertIn("lister_dual_clear_title_boxes(state->title_boxes[LISTER_DUAL_LEFT]);", lister_dual_c)
        self.assertIn("lister_dual_clear_title_boxes(state->title_boxes[LISTER_DUAL_RIGHT]);", lister_dual_c)
        self.assertIn("lister_dual_apply_title_boxes(lister, state, side);", apply_panel)
        self.assertRegex(
            draw_panel,
            r"(?s)TitleBox old_title_boxes\[16\];.*"
            r"CopyMem\(lister->title_boxes, old_title_boxes, sizeof\(old_title_boxes\)\);.*"
            r"lister_dual_clear_title_boxes\(lister->title_boxes\);.*"
            r"lister_display_dir\(lister\);.*"
            r"lister_dual_capture_title_boxes\(lister, state, side\);.*"
            r"CopyMem\(old_title_boxes, lister->title_boxes, sizeof\(old_title_boxes\)\);",
        )

    def test_dual_lister_has_no_linked_window_fallback(self):
        lister_h = read_program_source("lister.h")
        lister_dual_c = read_program_source_if_exists("lister_dual.c")
        lister_display_c = read_program_source("lister_display.c")
        lister_function_c = read_program_source("lister_function.c")
        lister_proc_c = read_program_source("lister_proc.c")
        function_launch_c = read_program_source("function_launch.c")
        function_run_c = read_program_source("function_run.c")

        for source in (lister_h, lister_dual_c, lister_display_c, lister_function_c, lister_proc_c,
                       function_launch_c, function_run_c):
            self.assertNotIn("lister_dual_partner", source)

        self.assertNotIn("linked in dual mode", lister_h)
        self.assertNotIn("dual lister partner", lister_proc_c)
        self.assertNotIn("lister_dual_get_partner", lister_dual_c)
        self.assertNotIn("lister_dual_get_owner", lister_dual_c)
        self.assertNotIn("lister_dual_position_partner", lister_dual_c)
        self.assertNotIn("lister_dual_position_owner", lister_dual_c)
        self.assertNotIn("lister_dual_sync_owner", lister_dual_c)
        self.assertNotIn("lister_dual_sync_partner", lister_dual_c)
        self.assertNotIn("LISTERF2_DUAL_SYNC", lister_dual_c)
        self.assertNotIn("state->partner", lister_dual_c)
        self.assertNotIn("state->owner", lister_dual_c)

    def test_dual_lister_role_menu_actions_stay_local_to_pair(self):
        lister_h = read_program_source("lister.h")
        lister_dual_c = read_program_source_if_exists("lister_dual.c")
        lister_function_c = read_program_source("lister_function.c")

        self.assertIn("void lister_dual_lock_source(Lister *lister);", lister_h)
        self.assertIn("void lister_dual_unlock(Lister *lister);", lister_h)
        self.assertRegex(
            lister_dual_c,
            r"(?s)static void lister_dual_set_roles\(Lister \*source, BOOL locked\).*"
            r"state = lister_dual_get_state\(source\).*"
            r"state->flags & DUALF_ONE_WINDOW.*"
            r"state->locked = locked;.*"
            r"lister_dual_apply_panel\(source, state->active\);.*"
            r"lister_dual_refresh\(source, LREFRESH_FULL\);",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)void lister_dual_activate\(Lister \*lister\).*"
            r"state = lister_dual_get_state\(lister\).*"
            r"state->flags & DUALF_ONE_WINDOW.*"
            r"lister_dual_apply_panel\(lister, state->active\);",
        )
        activate_match = re.search(
            r"(?s)\nvoid lister_dual_activate\(Lister \*lister\)\n\{.*?"
            r"\n}\n\n",
            lister_dual_c,
        )
        self.assertIsNotNone(activate_match)
        activate_func = activate_match.group(0)
        self.assertNotIn("BOOL locked", activate_func)
        self.assertNotIn("lister_dual_set_roles(lister, locked)", activate_func)
        self.assertRegex(
            lister_dual_c,
            r"(?s)void lister_dual_unlock\(Lister \*lister\).*"
            r"state = lister_dual_get_state\(lister\).*"
            r"state->locked = FALSE;.*"
            r"lister_dual_apply_panel\(lister, state->active\);.*"
            r"lister_dual_refresh\(lister, LREFRESH_FULL\);",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)void lister_dual_leave\(Lister \*lister\).*"
            r"lister->flags &= ~\(LISTERF_DEST \| LISTERF_STORED_SOURCE \| "
            r"LISTERF_STORED_DEST \| LISTERF_SOURCEDEST_LOCK\);.*"
            r"lister_fix_menus\(lister, 0\);",
        )
        self.assertRegex(
            lister_function_c,
            r"(?s)case MENU_LISTER_LOCK_SOURCE:.*"
            r"if \(lister_dual_is_side\(lister\)\).*"
            r"lister_dual_lock_source\(lister\);.*"
            r"case MENU_LISTER_LOCK_DEST:.*"
            r"if \(lister_dual_is_side\(lister\)\).*"
            r"lister_dual_activate_other\(lister\);.*"
            r"lister_dual_lock_source\(lister\);.*"
            r"case MENU_LISTER_UNLOCK:.*"
            r"if \(lister_dual_is_side\(lister\)\).*"
            r"lister_dual_unlock\(lister\);.*"
            r"case MENU_LISTER_OFF:.*"
            r"if \(lister_dual_is_side\(lister\)\).*"
            r"lister_dual_unlock\(lister\);",
        )

    def test_dual_lister_default_open_and_function_resolution_are_wired(self):
        event_loop_c = read_program_source("event_loop.c")
        function_devicelist_c = read_program_source("function_devicelist.c")
        lister_proc_c = read_program_source("lister_proc.c")
        function_launch_c = read_program_source("function_launch.c")
        function_run_c = read_program_source("function_run.c")
        lister_activate_c = read_program_source("lister_activate.c")

        self.assertIn("LISTEROPTF_DUAL_DEFAULT", event_loop_c)
        self.assertIn("LISTERF_DUAL", event_loop_c)
        self.assertIn("lister_launch_special_pair", lister_proc_c)
        self.assertRegex(
            lister_proc_c,
            r"(?s)if \(open && data\).*"
            r"if \(flags & LISTERF_DUAL\)\s+"
            r"lister_dual_enter\(lister\);.*"
            r"lister_open\(lister, \(struct Screen \*\)data\);",
        )
        self.assertRegex(
            lister_proc_c,
            r"(?s)case IPC_SHOW:.*"
            r"case LISTER_OPEN:.*"
            r"if \(flags & LISTERF_DUAL\)\s+"
            r"lister_dual_enter\(lister\);.*"
            r"lister_open\(lister, \(struct Screen \*\)data\);",
        )
        self.assertRegex(
            lister_proc_c,
            r"(?s)if \(flags & LISTERF_DUAL\)\s+"
            r"lister_dual_enter\(lister\);.*"
            r"lister_launch_special_pair\(lister, func\);",
        )
        self.assertRegex(
            lister_proc_c,
            r"(?s)if \(flags & LISTERF_DEVICES\).*"
            r"if \(flags & LISTERF_DUAL\)\s+"
            r"lister_dual_enter\(lister\);.*"
            r"lister_launch_special_pair\(lister, def_function_devicelist\);",
        )
        self.assertIn("LISTEROPTF_DUAL_DEFAULT", function_devicelist_c)
        self.assertRegex(
            function_devicelist_c,
            r"(?s)ULONG init_flags = 0;.*"
            r"environment->env->lister_options & LISTEROPTF_DUAL_DEFAULT.*"
            r"init_flags \|= LISTERF_DUAL;.*"
            r"IPC_Command\(lister->ipc,\s*LISTER_INIT,\s*init_flags",
        )
        self.assertIn("lister_dual_enter(lister)", lister_proc_c)
        self.assertNotIn("lister_dual_partner(source_list)", function_launch_c)
        self.assertIn("handle->dest_path", function_launch_c)
        self.assertIn("lister_dual_is_side(lister)", function_run_c)
        self.assertIn("function_dual_special_function", function_run_c)
        self.assertRegex(
            function_run_c,
            r"(?s)function_dual_special_function.*"
            r"ULONG \*open_flags.*"
            r"FUNC_DEVICELIST.*"
            r"def_function_devicelist_full.*"
            r"def_function_devicelist_brief.*"
            r"\*open_flags \|= LISTERF_DEVICES.*"
            r"def_function_devicelist.*"
            r"FUNC_BUFFERLIST.*"
            r"def_function_cachelist",
        )
        self.assertRegex(
            function_run_c,
            r"(?s)FUNCF_CREATE_SOURCE.*"
            r"function_dual_special_function\(handle, &special_open_flags\);.*"
            r"environment->env->lister_options & LISTEROPTF_DUAL_DEFAULT.*"
            r"open_flags \|= special_open_flags;.*"
            r"open_flags \|= LISTERF_DUAL;.*"
            r"IPC_Command\(lister->ipc,\s*LISTER_OPEN,\s*open_flags",
        )
        self.assertNotIn("Cfg_Function *dual_special;", function_run_c)
        self.assertNotIn("lister_dual_partner(lister)", function_run_c)
        self.assertIn("!lister_dual_is_side(lister)", lister_activate_c)

    def test_dual_lister_default_reaches_path_lister_initialisation(self):
        lister_proc_c = read_program_source("lister_proc.c")
        backdrop_open_c = read_program_source("backdrop_open.c")
        dirlist_read_c = read_program_source("dirlist_read.c")

        self.assertRegex(
            lister_proc_c,
            r"(?s)case LISTER_INIT:.*if \(flags & LISTERF_DUAL\)\s+lister_dual_enter\(lister\);",
        )
        self.assertIn("LISTEROPTF_DUAL_DEFAULT", backdrop_open_c)
        self.assertIn("LISTERF_DUAL", backdrop_open_c)
        self.assertIn("LISTEROPTF_DUAL_DEFAULT", dirlist_read_c)
        self.assertIn("LISTERF_DUAL", dirlist_read_c)
        self.assertIn("!(mode & LISTERMODE_ICON)", dirlist_read_c)

    def test_dual_toolbar_actions_target_active_side(self):
        lister_h = read_program_source("lister.h")
        lister_dual_c = read_program_source_if_exists("lister_dual.c")
        lister_toolbar_c = read_program_source("lister_toolbar.c")

        self.assertIn("Lister *lister_dual_active_side(Lister *lister);", lister_h)
        self.assertRegex(
            lister_dual_c,
            r"(?s)Lister \*lister_dual_active_side\(Lister \*lister\).*"
            r"lister_dual_is_side\(lister\).*"
            r"return lister;",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)Lister \*lister_dual_active_side\(Lister \*lister\).*"
            r"ListerDualState \*state;.*"
            r"state = lister_dual_get_state\(lister\).*"
            r"state->flags & DUALF_ONE_WINDOW.*"
            r"lister_dual_apply_panel\(lister, state->active\);.*"
            r"return lister;",
        )
        self.assertRegex(
            lister_toolbar_c,
            r"function_launch_quick\(FUNCTION_RUN_FUNCTION,\s*function,\s*"
            r"lister_dual_active_side\(lister\)\);",
        )

    def test_dual_function_launch_snapshots_active_panel(self):
        lister_h = read_program_source("lister.h")
        lister_dual_c = read_program_source_if_exists("lister_dual.c")
        function_launch_h = read_program_source("function_launch.h")
        function_paths_c = read_program_source("function_paths.c")
        function_launch_c = read_program_source("function_launch.c")
        function_run_c = read_program_source("function_run.c")
        lister_idcmp_c = read_program_source("lister_idcmp.c")
        file_select_c = read_program_source("file_select.c")

        self.assertIn("short lister_dual_active_index(Lister *lister);", lister_h)
        self.assertIn("void lister_dual_apply_side(Lister *lister, short side);", lister_h)
        self.assertIn("short dual_side;", function_launch_h)
        self.assertIn("#define LISTNF_DUAL_SIDE", function_launch_h)
        self.assertIn("void function_capture_path_side(PathNode *);", function_launch_h)
        self.assertIn("void function_apply_path_side(PathNode *);", function_launch_h)
        self.assertRegex(
            lister_dual_c,
            r"(?s)short lister_dual_active_index\(Lister \*lister\).*"
            r"return state->active;.*"
            r"return -1;",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)void lister_dual_apply_side\(Lister \*lister, short side\).*"
            r"lister_dual_capture_active_buffer\(lister\);.*"
            r"side < 0 \|\| side >= LISTER_DUAL_SIDES.*"
            r"state->active = side;.*"
            r"lister_dual_apply_panel\(lister, side\);",
        )
        self.assertRegex(
            function_paths_c,
            r"(?s)PathNode \*function_add_path.*"
            r"function_init_path_node\(node\);.*"
            r"node->lister = lister;.*"
            r"function_capture_path_side\(node\);",
        )
        self.assertRegex(
            function_paths_c,
            r"(?s)void function_capture_path_side\(PathNode \*node\).*"
            r"lister_dual_active_index\(node->lister\).*"
            r"node->dual_side = side;.*"
            r"node->flags \|= LISTNF_DUAL_SIDE;.*"
            r"lister_dual_apply_side\(node->lister, side\);",
        )
        self.assertRegex(
            function_paths_c,
            r"(?s)PathNode \*function_path_current\(PathList \*list\).*"
            r"function_apply_path_side\(list->current\);.*"
            r"list->current->lister->cur_buffer->buf_Path;",
        )
        self.assertRegex(
            function_paths_c,
            r"(?s)Lister \*function_lister_current\(PathList \*list\).*"
            r"function_apply_path_side\(list->current\);.*"
            r"return list->current->lister;",
        )
        self.assertRegex(
            function_paths_c,
            r"(?s)BOOL function_valid_path\(PathNode \*path\).*"
            r"function_apply_path_side\(path\);.*"
            r"path->path = path->lister->cur_buffer->buf_Path;",
        )
        self.assertRegex(
            function_launch_c,
            r"(?s)BOOL function_lock_paths.*"
            r"function_apply_path_side\(node\);.*"
            r"lister = function_valid_lister\(handle, node->lister\);",
        )
        self.assertIn("function_capture_path_side(node);", function_launch_c)
        self.assertIn("function_capture_path_side(current);", function_run_c)
        self.assertIn("select_select_files(lister_dual_active_side(lister),", lister_idcmp_c)
        self.assertIn("select_rmb_scroll(lister_dual_active_side(lister),", lister_idcmp_c)
        self.assertRegex(
            file_select_c,
            r"(?s)void file_run_function\(Lister \*lister, DirEntry \*entry, Cfg_Function \*func, char \*dest, ULONG flags\).*"
            r"lister = lister_dual_active_side\(lister\);.*"
            r"lister_dual_suppress_popup_front\(lister\);",
        )

    def test_dual_side_targeted_refresh_ipc_preserves_active_side(self):
        lister_h = read_program_source("lister.h")
        lister_dual_c = read_program_source_if_exists("lister_dual.c")
        lister_proc_c = read_program_source("lister_proc.c")

        self.assertIn("BOOL lister_dual_apply_side_temporary(Lister *lister, short side);", lister_h)
        self.assertIn("void lister_dual_restore_active(Lister *lister, short side);", lister_h)
        self.assertRegex(
            lister_dual_c,
            r"(?s)BOOL lister_dual_apply_side_temporary\(Lister \*lister, short side\).*"
            r"lister_dual_capture_active_buffer\(lister\);.*"
            r"state->active = side;.*"
            r"lister_dual_apply_panel\(lister, side\);",
        )
        temporary_func = re.search(
            r"(?s)BOOL lister_dual_apply_side_temporary\(Lister \*lister, short side\).*?"
            r"\n}\n\nvoid lister_dual_restore_active",
            lister_dual_c,
        ).group(0)
        self.assertIn("state->active = side;", temporary_func)
        self.assertRegex(
            lister_dual_c,
            r"(?s)void lister_dual_restore_active\(Lister \*lister, short side\).*"
            r"state->buffer\[state->active\] = lister->cur_buffer;.*"
            r"lister_dual_capture_side_offsets\(lister, state, state->active\);.*"
            r"lister_dual_capture_title_boxes\(lister, state, state->active\);.*"
            r"state->active = side;.*"
            r"lister_dual_apply_panel\(lister, state->active\);",
        )
        self.assertRegex(
            lister_proc_c,
            r"(?s)static short lister_proc_apply_side\(Lister \*\*lister, ULONG side, BOOL preserve_active\).*"
            r"old_side = lister_dual_active_index\(\*lister\);.*"
            r"preserve_active && lister_dual_apply_side_temporary\(\*lister, side - 1\).*"
            r"return old_side;.*"
            r"lister_dual_apply_side\(\*lister, side - 1\);.*"
            r"\*lister = lister_dual_active_side\(\*lister\);",
        )
        self.assertRegex(
            lister_proc_c,
            r"(?s)static void lister_proc_restore_side\(Lister \*lister, short side\).*"
            r"if \(side >= 0\).*"
            r"lister_dual_restore_active\(lister, side\);",
        )

        for case_name, side_expr in (
            ("LISTER_REFRESH_PATH", "flags"),
            ("LISTER_REFRESH_FREE", "flags"),
            ("LISTER_REFRESH_WINDOW", "(IPTR)data"),
            ("LISTER_SHOW_INFO", "flags"),
            ("LISTER_UPDATE_STAMP", "flags"),
            ("LISTER_SET_GAUGE", "flags"),
        ):
            with self.subTest(case_name=case_name):
                block = re.search(r"(?s)case %s:.*?break;" % case_name, lister_proc_c).group(0)
                self.assertRegex(
                    block,
                    r"(?s)restore_side = lister_proc_apply_side\(&lister, %s, TRUE\);.*"
                    r"lister_proc_restore_side\(lister, restore_side\);" % re.escape(side_expr),
                )

    def test_dual_enter_and_role_changes_clear_external_source_dest_roles(self):
        lister_dual_c = read_program_source_if_exists("lister_dual.c")

        self.assertRegex(
            lister_dual_c,
            r"(?s)static void lister_dual_clear_external_roles\(Lister \*dual\).*"
            r"lock_listlock\(&GUI->lister_list, FALSE\);.*"
            r"for \(ipc = \(IPCData \*\)GUI->lister_list.list.lh_Head;.*"
            r"lister = IPCDATA\(ipc\);.*"
            r"if \(lister == dual \|\| lister_dual_is_side\(lister\)\).*"
            r"continue;.*"
            r"lister->flags & \(LISTERF_SOURCE \| LISTERF_DEST \| LISTERF_STORED_SOURCE \| "
            r"LISTERF_STORED_DEST \| LISTERF_SOURCEDEST_LOCK\).*"
            r"IPC_Command\(lister->ipc, LISTER_OFF, 0, 0, 0, 0\);.*"
            r"unlock_listlock\(&GUI->lister_list\);",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)BOOL lister_dual_enter\(Lister \*lister\).*"
            r"lister_dual_clear_external_roles\(lister\);.*"
            r"lister_check_source\(lister\);",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)static void lister_dual_set_roles\(Lister \*source, BOOL locked\).*"
            r"lister_dual_clear_external_roles\(source\);.*"
            r"lister_dual_apply_panel\(source, state->active\);",
        )

    def test_dual_filechanges_update_inactive_panel_buffer(self):
        function_filechange_c = read_program_source("function_filechange.c")
        function_launch_c = read_program_source("function_launch.c")
        function_support_c = read_program_source("function_support.c")
        lister_proc_c = read_program_source("lister_proc.c")

        self.assertRegex(
            function_filechange_c,
            r"(?s)void function_filechange_do\(FunctionHandle \*handle, BOOL strip\).*"
            r"old_side = lister_dual_active_index\(lister\);.*"
            r"side_count = LISTER_DUAL_SIDES;.*"
            r"for \(side = 0; side < side_count; side\+\+\).*"
            r"lister_dual_apply_side\(lister, side\);.*"
            r"lister_dual_apply_side\(lister, old_side\);",
        )
        self.assertIn("ref_flags |= REFRESHF_FULL;", function_filechange_c)
        self.assertRegex(
            function_filechange_c,
            r"(?s)for \(side = 0; side < side_count; side\+\+\).*"
            r"BOOL show;.*"
            r"ULONG ref_flags;.*"
            r"DirEntry \*show_file;.*"
            r"if \(show\).*"
            r"if \(old_side != -1 \|\| lister->locker_ipc != handle->ipc \|\| !handle->ipc\).*"
            r"refresh_side = \(old_side != -1\) \? side \+ 1 : 0;.*"
            r"IPC_Command\(lister->ipc,\s*"
            r"LISTER_REFRESH_WINDOW,\s*"
            r"ref_flags \| REFRESHF_DATESTAMP,\s*"
            r"\(APTR\)\(IPTR\)refresh_side,",
        )
        self.assertNotIn("old_side + 1 : 0", function_filechange_c)
        refresh_block = re.search(r"(?s)case LISTER_REFRESH_WINDOW:.*?break;", lister_proc_c).group(0)
        self.assertIn("restore_side = lister_proc_apply_side(&lister, (IPTR)data, TRUE);", refresh_block)
        self.assertIn("lister_proc_restore_side(lister, restore_side);", refresh_block)
        self.assertNotIn("ref |= REFRESHF_FULL;", function_launch_c)
        self.assertRegex(
            function_support_c,
            r"(?s)void function_cleanup\(FunctionHandle \*handle, PathNode \*node, BOOL full\).*"
            r"function_apply_path_side\(node\);.*"
            r"if \(\(lister = node->lister\)\)",
        )

    def test_dual_function_status_helpers_apply_source_panel(self):
        function_support_c = read_program_source("function_support.c")
        status_helpers = (
            ("function_abort", "status_abort\\(node->lister\\);"),
            ("function_error_text", "status_display_error\\(node->lister, code\\);"),
            ("function_text", "status_text\\(node->lister, text\\);"),
        )

        for helper_name, status_call in status_helpers:
            with self.subTest(helper_name=helper_name):
                self.assertRegex(
                    function_support_c,
                    r"(?s)void %s\(FunctionHandle \*handle.*?"
                    r"function_apply_path_side\(node\);.*"
                    r"%s" % (helper_name, status_call),
                )

    def test_dual_global_path_discovery_skips_dual_listers(self):
        function_launch_c = read_program_source("function_launch.c")
        function_get_paths = re.search(
            r"(?s)Lister \*function_get_paths\(FunctionHandle \*handle, PathList \*list, ULONG flags, short one_only\).*",
            function_launch_c,
        ).group(0)

        self.assertRegex(
            function_get_paths,
            r"(?s)// Go through lister list.*"
            r"lister = IPCDATA\(ipc\);.*"
            r"if \(lister_dual_is_side\(lister\)\)\s*"
            r"continue;.*"
            r"// Is this lister the right type",
        )

    def test_dual_function_file_list_uses_captured_source_panel(self):
        function_files_c = read_program_source("function_files.c")

        self.assertRegex(
            function_files_c,
            r"(?s)int function_build_list\(FunctionHandle \*handle, PathNode \*\*path, InstructionParsed \*instruction\).*"
            r"function_apply_path_side\(\*path\);.*"
            r"\(\*path\)->lister->cur_buffer",
        )
        self.assertRegex(
            function_files_c,
            r"(?s)FunctionEntry \*function_get_entry\(FunctionHandle \*handle\).*"
            r"function_apply_path_side\(handle->source_paths.current\);.*"
            r"handle->source_paths.current->lister->cur_buffer",
        )

    def test_dual_path_change_ipc_carries_captured_side(self):
        function_support_c = read_program_source("function_support.c")
        lister_proc_c = read_program_source("lister_proc.c")

        self.assertRegex(
            function_support_c,
            r"(?s)void function_perform_changes\(FunctionHandle \*handle, PathNode \*path\).*"
            r"side = \(path->flags & LISTNF_DUAL_SIDE\) \? path->dual_side \+ 1 : 0;.*"
            r"IPC_Command\(path->lister->ipc, LISTER_RESCAN, side, 0, 0, 0\);.*"
            r"IPC_Command\(path->lister->ipc, LISTER_UPDATE_STAMP, side, 0, 0, 0\);",
        )
        rescan_block = re.search(r"(?s)case LISTER_RESCAN:.*?break;", lister_proc_c).group(0)
        self.assertRegex(
            rescan_block,
            r"(?s)if \(flags > 0 && flags <= LISTER_DUAL_SIDES\).*"
            r"lister_dual_apply_side\(lister, flags - 1\);.*"
            r"else.*"
            r"lister = lister_dual_active_side\(lister\);",
        )
        stamp_block = re.search(r"(?s)case LISTER_UPDATE_STAMP:.*?break;", lister_proc_c).group(0)
        self.assertRegex(
            stamp_block,
            r"(?s)restore_side = lister_proc_apply_side\(&lister, flags, TRUE\);.*"
            r"lister_proc_restore_side\(lister, restore_side\);",
        )

    def test_dual_unlock_refresh_ipc_carries_captured_side(self):
        function_launch_c = read_program_source("function_launch.c")
        lister_proc_c = read_program_source("lister_proc.c")

        self.assertRegex(
            function_launch_c,
            r"(?s)void function_unlock_paths\(FunctionHandle \*handle, PathList \*list, int locker\).*"
            r"side = \(node->flags & LISTNF_DUAL_SIDE\) \? node->dual_side \+ 1 : 0;.*"
            r"IPC_Command\(node->lister->ipc, LISTER_REFRESH_WINDOW, ref, \(APTR\)\(IPTR\)side, 0, 0\);.*"
            r"IPC_Command\(node->lister->ipc, LISTER_SHOW_INFO, side, 0, 0, 0\);",
        )
        refresh_block = re.search(r"(?s)case LISTER_REFRESH_WINDOW:.*?break;", lister_proc_c).group(0)
        self.assertRegex(
            refresh_block,
            r"(?s)restore_side = lister_proc_apply_side\(&lister, \(IPTR\)data, TRUE\);.*"
            r"lister_proc_restore_side\(lister, restore_side\);",
        )
        show_info_block = re.search(r"(?s)case LISTER_SHOW_INFO:.*?break;", lister_proc_c).group(0)
        self.assertRegex(
            show_info_block,
            r"(?s)restore_side = lister_proc_apply_side\(&lister, flags, TRUE\);.*"
            r"lister_proc_restore_side\(lister, restore_side\);",
        )

    def test_dual_refresh_name_ipc_carries_captured_side(self):
        function_copy_c = read_program_source("function_copy.c")
        function_delete_c = read_program_source("function_delete.c")
        lister_proc_c = read_program_source("lister_proc.c")

        for source_name, source in (
            ("function_copy.c", function_copy_c),
            ("function_delete.c", function_delete_c),
        ):
            with self.subTest(source_name=source_name):
                self.assertRegex(
                    source,
                    r"(?s)side = \(path->flags & LISTNF_DUAL_SIDE\) \? path->dual_side \+ 1 : 0;.*"
                    r"IPC_Command\(path->lister->ipc, LISTER_REFRESH_NAME, side, 0, 0, 0\);",
                )

        refresh_name_block = re.search(r"(?s)case LISTER_REFRESH_FREE:.*?break;", lister_proc_c).group(0)
        self.assertRegex(
            refresh_name_block,
            r"(?s)restore_side = lister_proc_apply_side\(&lister, flags, TRUE\);.*"
            r"lister_proc_restore_side\(lister, restore_side\);",
        )

    def test_dual_keyboard_and_lister_menu_actions_target_active_side(self):
        lister_idcmp_c = read_program_source("lister_idcmp.c")

        self.assertEqual(
            lister_idcmp_c.count(
                "function_launch_quick(FUNCTION_RUN_FUNCTION, function, lister_dual_active_side(lister));"
            ),
            2,
        )
        self.assertIn(
            "function_launch_quick(FUNCTION_RUN_FUNCTION, def_function_select, lister_dual_active_side(lister));",
            lister_idcmp_c,
        )
        self.assertIn(
            "function_launch_quick(FUNCTION_RUN_FUNCTION, def_function_devicelist, lister_dual_active_side(lister));",
            lister_idcmp_c,
        )
        self.assertIn(
            "function_launch_quick(FUNCTION_RUN_FUNCTION, def_function_cachelist, lister_dual_active_side(lister));",
            lister_idcmp_c,
        )
        self.assertNotRegex(
            lister_idcmp_c,
            r"function_launch_quick\(FUNCTION_RUN_FUNCTION,\s*"
            r"(function|def_function_select|def_function_devicelist|def_function_cachelist),\s*lister\);",
        )

    def test_dual_lister_menu_functions_start_from_active_side(self):
        lister_function_c = read_program_source("lister_function.c")

        self.assertRegex(
            lister_function_c,
            r"(?s)int lister_do_function\(Lister \*lister, IPTR func\).*"
            r"lister = lister_dual_active_side\(lister\);.*"
            r"// Not special buffer or icons",
        )

    def test_dual_keyboard_direct_selection_shortcuts_target_active_side(self):
        lister_idcmp_c = read_program_source("lister_idcmp.c")

        self.assertIn("select_global_state(lister_dual_active_side(lister), 1);", lister_idcmp_c)
        self.assertIn("select_global_state(lister_dual_active_side(lister), 0);", lister_idcmp_c)
        self.assertIn("select_global_toggle(lister_dual_active_side(lister));", lister_idcmp_c)
        self.assertRegex(
            lister_idcmp_c,
            r"(?s)// \\ does reselect.*"
            r"lister = lister_dual_active_side\(lister\);.*"
            r"DoReselect\(&lister->reselect, lister, 1\);",
        )
        self.assertNotIn("select_global_state(lister, 1);", lister_idcmp_c)
        self.assertNotIn("select_global_state(lister, 0);", lister_idcmp_c)
        self.assertNotIn("select_global_toggle(lister);", lister_idcmp_c)

    def test_dual_lister_proc_selection_ipc_targets_active_side(self):
        lister_proc_c = read_program_source("lister_proc.c")
        selection_cases = (
            "LISTER_MAKE_RESELECT",
            "LISTER_DO_RESELECT",
            "LISTER_FIND_FIRST_SEL",
            "LISTER_FIND_ENTRY",
            "LISTER_REMOVE_SIZES",
            "LISTER_SELECT_GLOBAL_STATE",
            "LISTER_SELECT_GLOBAL_TOGGLE",
            "LISTER_SELECT_WILD",
        )

        for case_name in selection_cases:
            with self.subTest(case_name=case_name):
                block = re.search(r"(?s)case %s:.*?break;" % case_name, lister_proc_c).group(0)
                self.assertIn("lister = lister_dual_active_side(lister);", block)

    def test_dual_all_none_toggle_selection_ipc_snapshots_active_side(self):
        lister_proc_c = read_program_source("lister_proc.c")
        function_all_c = read_program_source("function_all.c")
        function_none_c = read_program_source("function_none.c")
        function_toggle_c = read_program_source("function_toggle.c")

        for source in (function_all_c, function_none_c, function_toggle_c):
            with self.subTest(source=source.splitlines()[0]):
                self.assertIn("side = 0;", source)
                self.assertRegex(
                    source,
                    r"(?s)if \(\(active = lister_dual_active_index\(lister\)\) >= 0\)\s*"
                    r"side = active \+ 1;",
                )
                self.assertRegex(
                    source,
                    r"IPC_Command\(lister->ipc, LISTER_REFRESH_WINDOW, 0, \(APTR\)\(IPTR\)side, 0, REPLY_NO_PORT\);",
                )

        self.assertRegex(
            function_all_c,
            r"IPC_Command\(lister->ipc, LISTER_SELECT_GLOBAL_STATE, 1, \(APTR\)\(IPTR\)side, 0, REPLY_NO_PORT\);",
        )
        self.assertRegex(
            function_none_c,
            r"IPC_Command\(lister->ipc, LISTER_SELECT_GLOBAL_STATE, 0, \(APTR\)\(IPTR\)side, 0, REPLY_NO_PORT\);",
        )
        self.assertRegex(
            function_toggle_c,
            r"IPC_Command\(lister->ipc, LISTER_SELECT_GLOBAL_TOGGLE, side, 0, 0, REPLY_NO_PORT\);",
        )

        state_block = re.search(r"(?s)case LISTER_SELECT_GLOBAL_STATE:.*?break;", lister_proc_c).group(0)
        toggle_block = re.search(r"(?s)case LISTER_SELECT_GLOBAL_TOGGLE:.*?break;", lister_proc_c).group(0)
        self.assertRegex(
            state_block,
            r"(?s)if \(\(IPTR\)data > 0 && \(IPTR\)data <= LISTER_DUAL_SIDES\).*"
            r"lister_dual_apply_side\(lister, \(IPTR\)data - 1\);.*"
            r"else.*"
            r"lister = lister_dual_active_side\(lister\);.*"
            r"select_global_state\(lister, flags\);",
        )
        self.assertRegex(
            toggle_block,
            r"(?s)if \(flags > 0 && flags <= LISTER_DUAL_SIDES\).*"
            r"lister_dual_apply_side\(lister, flags - 1\);.*"
            r"else.*"
            r"lister = lister_dual_active_side\(lister\);.*"
            r"select_global_toggle\(lister\);",
        )

    def test_dual_select_wild_ipc_snapshots_active_side(self):
        function_launch_h = read_program_source("function_launch.h")
        function_select_c = read_program_source("function_select.c")
        lister_proc_c = read_program_source("lister_proc.c")

        self.assertIn("} SelectWildData;", function_launch_h)
        self.assertIn("SelectData data;", function_launch_h)
        self.assertIn("PathList *dest_list;", function_launch_h)
        self.assertIn("ULONG side;", function_launch_h)
        self.assertRegex(
            function_select_c,
            r"(?s)DOPUS_FUNC\(function_select\).*"
            r"SelectWildData \*packet;.*"
            r"side = 0;.*"
            r"if \(\(active = lister_dual_active_index\(lister\)\) >= 0\)\s*"
            r"side = active \+ 1;.*"
            r"packet = AllocVec\(sizeof\(SelectWildData\), MEMF_CLEAR\).*"
            r"CopyMem\(data, &packet->data, sizeof\(SelectData\)\);.*"
            r"packet->dest_list = &handle->dest_paths;.*"
            r"packet->side = side;.*"
            r"if \(!IPC_Command\(lister->ipc, LISTER_SELECT_WILD, 0, packet, packet, REPLY_NO_PORT\)\).*"
            r"FreeVec\(packet\);.*"
            r"else.*"
            r"IPC_Command\(lister->ipc, LISTER_SELECT_WILD, \(IPTR\)&handle->dest_paths, data, 0, REPLY_NO_PORT\);.*"
            r"IPC_Command\(lister->ipc, LISTER_REFRESH_WINDOW, 0, \(APTR\)\(IPTR\)side, 0, REPLY_NO_PORT\);",
        )
        self.assertRegex(
            function_select_c,
            r"(?s)void function_select_file\(FunctionHandle \*handle, Lister \*lister, char \*filename\).*"
            r"SelectWildData \*packet;.*"
            r"side = 0;.*"
            r"if \(\(active = lister_dual_active_index\(lister\)\) >= 0\)\s*"
            r"side = active \+ 1;.*"
            r"packet = AllocVec\(sizeof\(SelectWildData\), MEMF_CLEAR\).*"
            r"CopyMem\(&data, &packet->data, sizeof\(SelectData\)\);.*"
            r"packet->dest_list = &handle->dest_paths;.*"
            r"packet->side = side;.*"
            r"if \(!IPC_Command\(lister->ipc, LISTER_SELECT_WILD, 0, packet, packet, \(struct MsgPort \*\)-1\)\).*"
            r"FreeVec\(packet\);.*"
            r"else.*"
            r"IPC_Command\(lister->ipc, LISTER_SELECT_WILD, \(IPTR\)&handle->dest_paths, &data, 0, \(struct MsgPort \*\)-1\);",
        )

        wild_block = re.search(r"(?s)case LISTER_SELECT_WILD:.*?break;", lister_proc_c).group(0)
        self.assertRegex(
            wild_block,
            r"(?s)SelectWildData \*select = \(SelectWildData \*\)data;.*"
            r"if \(select && lmsg->data_free == select\).*"
            r"if \(select->side > 0 && select->side <= LISTER_DUAL_SIDES\).*"
            r"lister_dual_apply_side\(lister, select->side - 1\);.*"
            r"else.*"
            r"lister = lister_dual_active_side\(lister\);.*"
            r"select_global_wild\(lister, &select->data, select->dest_list\);.*"
            r"else.*"
            r"select_global_wild\(lister, \(SelectData \*\)data, \(PathList \*\)flags\);",
        )

    def test_dual_reselect_ipc_snapshots_active_side(self):
        function_launch_h = read_program_source("function_launch.h")
        function_reselect_c = read_program_source("function_reselect.c")
        function_readdir_c = read_program_source("function_readdir.c")
        lister_proc_c = read_program_source("lister_proc.c")

        self.assertIn("} ListerReselectData;", function_launch_h)
        self.assertIn("ReselectionData *reselect;", function_launch_h)
        self.assertIn("ULONG flags;", function_launch_h)
        self.assertIn("ULONG side;", function_launch_h)

        self.assertRegex(
            function_reselect_c,
            r"(?s)DOPUS_FUNC\(function_reselect\).*"
            r"ListerReselectData \*packet;.*"
            r"side = 0;.*"
            r"if \(\(active = lister_dual_active_index\(lister\)\) >= 0\)\s*"
            r"side = active \+ 1;.*"
            r"packet = AllocVec\(sizeof\(ListerReselectData\), MEMF_CLEAR\).*"
            r"packet->reselect = &lister->reselect;.*"
            r"packet->flags = 1;.*"
            r"packet->side = side;.*"
            r"if \(!IPC_Command\(lister->ipc, LISTER_DO_RESELECT, 0, packet, packet, \(struct MsgPort \*\)-1\)\).*"
            r"FreeVec\(packet\);",
        )
        self.assertRegex(
            function_readdir_c,
            r"(?s)void function_read_directory\(FunctionHandle \*handle, Lister \*lister, char \*source_path\).*"
            r"ListerReselectData \*packet;.*"
            r"side = 0;.*"
            r"if \(\(active = lister_dual_active_index\(lister\)\) >= 0\)\s*"
            r"side = active \+ 1;.*"
            r"packet = AllocVec\(sizeof\(ListerReselectData\), MEMF_CLEAR\).*"
            r"packet->reselect = &reselect;.*"
            r"packet->flags = RESELF_SAVE_FILETYPES;.*"
            r"packet->side = side;.*"
            r"if \(!IPC_Command\(lister->ipc, LISTER_MAKE_RESELECT, 0, packet, packet, REPLY_NO_PORT\)\).*"
            r"FreeVec\(packet\);.*"
            r"packet = AllocVec\(sizeof\(ListerReselectData\), MEMF_CLEAR\).*"
            r"packet->reselect = &reselect;.*"
            r"packet->flags = 0;.*"
            r"packet->side = side;.*"
            r"if \(!IPC_Command\(lister->ipc, LISTER_DO_RESELECT, 0, packet, packet, REPLY_NO_PORT\)\).*"
            r"FreeVec\(packet\);",
        )

        for case_name, call in (
            ("LISTER_MAKE_RESELECT", r"MakeReselect\(select->reselect, lister->cur_buffer, select->flags\);"),
            ("LISTER_DO_RESELECT", r"DoReselect\(select->reselect, lister, select->flags\);"),
        ):
            with self.subTest(case_name=case_name):
                block = re.search(r"(?s)case %s:.*?break;" % case_name, lister_proc_c).group(0)
                self.assertRegex(
                    block,
                    r"(?s)ListerReselectData \*select = \(ListerReselectData \*\)data;.*"
                    r"if \(select && lmsg->data_free == select\).*"
                    r"if \(select->side > 0 && select->side <= LISTER_DUAL_SIDES\).*"
                    r"lister_dual_apply_side\(lister, select->side - 1\);.*"
                    r"else.*"
                    r"lister = lister_dual_active_side\(lister\);.*"
                    + call
                    + r".*else.*"
                    r"lister = lister_dual_active_side\(lister\);",
                )

    def test_dual_read_directory_ipc_snapshots_active_side(self):
        function_launch_h = read_program_source("function_launch.h")
        function_readdir_c = read_program_source("function_readdir.c")
        lister_proc_c = read_program_source("lister_proc.c")

        self.assertIn("} ListerBufferFindData;", function_launch_h)
        self.assertIn("target a specific panel", function_launch_h)
        self.assertIn("char *path;", function_launch_h)
        self.assertIn("struct DateStamp *stamp;", function_launch_h)
        self.assertIn("char *name;", function_launch_h)
        self.assertIn("ULONG side;", function_launch_h)

        self.assertRegex(
            function_readdir_c,
            r"(?s)void function_read_directory\(FunctionHandle \*handle, Lister \*lister, char \*source_path\).*"
            r"ListerBufferFindData \*find_packet;.*"
            r"side = 0;.*"
            r"if \(\(active = lister_dual_active_index\(lister\)\) >= 0\)\s*"
            r"side = active \+ 1;.*"
            r"IPC_Command\(lister->ipc, LISTER_CHECK_SPECIAL_BUFFER, side, 0, 0, REPLY_NO_PORT\);.*"
            r"find_packet = AllocVec\(sizeof\(ListerBufferFindData\), MEMF_CLEAR\).*"
            r"find_packet->path = PATH_FULL_DEVICE;.*"
            r"find_packet->stamp = stamp_ptr;.*"
            r"find_packet->side = side;.*"
            r"IPC_Command\(\s*lister->ipc, LISTER_BUFFER_FIND, 0, find_packet, find_packet, REPLY_NO_PORT\).*"
            r"IPC_Command\(lister->ipc, LISTER_SHOW_BUFFER, LISTER_SHOW_BUFFER_PACK_FLAGS\(TRUE, side\), buffer, 0, REPLY_NO_PORT\);.*"
            r"IPC_Command\(\s*lister->ipc, LISTER_BUFFER_FIND_EMPTY, 0, find_packet, find_packet, REPLY_NO_PORT\).*"
            r"IPC_Command\(lister->ipc, LISTER_REFRESH_PATH, side, 0, 0, 0\);.*"
            r"IPC_Command\(lister->ipc,\s*LISTER_REFRESH_WINDOW,\s*REFRESHF_UPDATE_NAME \| REFRESHF_STATUS \| REFRESHF_SLIDERS \| REFRESHF_CLEAR_ICONS,\s*"
            r"\(APTR\)\(IPTR\)side,.*"
            r"IPC_Command\(lister->ipc,\s*LISTER_REFRESH_WINDOW,\s*REFRESHF_UPDATE_NAME \| REFRESHF_STATUS \| REFRESHF_SLIDERS \| REFRESHF_CD,\s*"
            r"\(APTR\)\(IPTR\)side,.*"
            r"IPC_Command\(lister->ipc, LISTER_SET_GAUGE, side, 0, 0, 0\);",
        )
        self.assertNotIn("find_packet = 0;", function_readdir_c)
        self.assertNotRegex(
            function_readdir_c,
            r"if \(!IPC_Command\(\s*lister->ipc, LISTER_BUFFER_FIND_EMPTY, 0, find_packet, find_packet, REPLY_NO_PORT\)\)",
        )

        for case_name, call in (
            ("LISTER_BUFFER_FIND", r"lister_find_buffer\(lister,\s*0,\s*find->path,\s*find->stamp,\s*find->name,"),
            ("LISTER_BUFFER_FIND_EMPTY", r"lister_buffer_find_empty\(lister, find->path, find->stamp\);"),
        ):
            with self.subTest(case_name=case_name):
                block = re.search(r"(?s)case %s:.*?break;" % case_name, lister_proc_c).group(0)
                self.assertRegex(
                    block,
                    r"(?s)ListerBufferFindData \*find = \(ListerBufferFindData \*\)data;.*"
                    r"if \(find && lmsg->data_free == find\).*"
                    r"if \(find->side > 0 && find->side <= LISTER_DUAL_SIDES\).*"
                    r"lister_dual_apply_side\(lister, find->side - 1\);.*"
                    r"else.*"
                    r"lister = lister_dual_active_side\(lister\);.*"
                    + call,
                )

        special_block = re.search(r"(?s)case LISTER_CHECK_SPECIAL_BUFFER:.*?break;", lister_proc_c).group(0)
        refresh_path_block = re.search(r"(?s)case LISTER_REFRESH_PATH:.*?break;", lister_proc_c).group(0)
        set_gauge_block = re.search(r"(?s)case LISTER_SET_GAUGE:.*?break;", lister_proc_c).group(0)
        show_buffer_block = re.search(r"(?s)case LISTER_SHOW_BUFFER:.*?break;", lister_proc_c).group(0)

        for block in (refresh_path_block, set_gauge_block):
            self.assertRegex(
                block,
                r"(?s)restore_side = lister_proc_apply_side\(&lister, flags, TRUE\);.*"
                r"lister_proc_restore_side\(lister, restore_side\);",
            )
        self.assertRegex(
            special_block,
            r"(?s)if \(flags > 0 && flags <= LISTER_DUAL_SIDES\).*"
            r"lister_dual_apply_side\(lister, flags - 1\);.*"
            r"else.*"
            r"lister = lister_dual_active_side\(lister\);",
        )
        self.assertRegex(
            show_buffer_block,
            r"(?s)side = LISTER_SHOW_BUFFER_SIDE\(flags\);.*"
            r"flags = LISTER_SHOW_BUFFER_BASE_FLAGS\(flags\);.*"
            r"if \(side > 0 && side <= LISTER_DUAL_SIDES\).*"
            r"lister_dual_apply_side\(lister, side - 1\);.*"
            r"else.*"
            r"lister = lister_dual_active_side\(lister\);.*"
            r"if \(flags\).*"
            r"unlock_listlock\(&GUI->buffer_list\);",
        )

    def test_dual_lister_proc_path_buffer_ipc_targets_active_side(self):
        lister_proc_c = read_program_source("lister_proc.c")
        path_buffer_cases = (
            "LISTER_CHECK_SPECIAL_BUFFER",
            "LISTER_SHOW_SPECIAL_BUFFER",
            "LISTER_GET_PATH",
            "LISTER_BUFFER_FIND",
            "LISTER_SHOW_BUFFER",
            "LISTER_BUFFER_FIND_EMPTY",
            "LISTER_FIND_CACHED_BUFFER",
            "LISTER_REFRESH_TITLE",
            "LISTER_REFRESH_SLIDERS",
            "LISTER_DO_PARENT_ROOT",
            "LISTER_COPY_BUFFER",
            "LISTER_RESCAN",
        )

        for case_name in path_buffer_cases:
            with self.subTest(case_name=case_name):
                block = re.search(r"(?s)case %s:.*?break;" % case_name, lister_proc_c).group(0)
                self.assertIn("lister = lister_dual_active_side(lister);", block)

    def test_dual_lister_proc_buffer_switch_ipc_persists_active_panel_buffer(self):
        lister_h = read_program_source("lister.h")
        lister_dual_c = read_program_source_if_exists("lister_dual.c")
        lister_proc_c = read_program_source("lister_proc.c")
        buffer_switch_cases = (
            "LISTER_CHECK_SPECIAL_BUFFER",
            "LISTER_SHOW_SPECIAL_BUFFER",
            "LISTER_SHOW_BUFFER",
            "LISTER_BUFFER_FIND_EMPTY",
        )

        self.assertIn("void lister_dual_store_active_buffer(Lister *lister);", lister_h)
        self.assertRegex(
            lister_dual_c,
            r"(?s)void lister_dual_store_active_buffer\(Lister \*lister\).*"
            r"lister_dual_capture_active_buffer\(lister\);.*"
            r"lister_dual_apply_panel\(lister, state->active\);",
        )
        for case_name in buffer_switch_cases:
            with self.subTest(case_name=case_name):
                block = re.search(r"(?s)case %s:.*?break;" % case_name, lister_proc_c).group(0)
                self.assertIn("lister_dual_store_active_buffer(lister);", block)

    def test_dual_keyboard_and_mouse_configure_targets_active_side(self):
        lister_idcmp_c = read_program_source("lister_idcmp.c")

        self.assertIn("lister_configure(lister_dual_active_side(lister));", lister_idcmp_c)
        self.assertNotIn("lister_configure(lister);", lister_idcmp_c)

    def test_dual_rawkey_handler_uses_active_panel_state(self):
        lister_idcmp_c = read_program_source("lister_idcmp.c")

        self.assertRegex(
            lister_idcmp_c,
            r"(?s)case IDCMP_RAWKEY:.*"
            r"lister = lister_dual_active_side\(lister\);.*"
            r"if \(lister_edit_key\(lister, msg\)\).*"
            r"buffer = lister->cur_buffer;",
        )

    def test_dual_tab_key_switches_panels_before_global_listers(self):
        lister_idcmp_c = read_program_source("lister_idcmp.c")

        self.assertRegex(
            lister_idcmp_c,
            r"(?s)// Tab activates.*"
            r"case 0x42:.*"
            r"if \(lister_dual_one_window\(lister\)\).*"
            r"lister_dual_activate_other\(lister\);.*"
            r"break;.*"
            r"lock_listlock\(&GUI->lister_list, FALSE\);",
        )

    def test_dual_lister_restores_per_panel_lister_menu_button(self):
        lister_h = read_program_source("lister.h")
        lister_dual_c = read_program_source_if_exists("lister_dual.c")
        lister_idcmp_c = read_program_source("lister_idcmp.c")

        self.assertIn("BOOL lister_dual_popup_at(Lister *lister, UWORD x, UWORD y);", lister_h)
        self.assertIn("void lister_dual_suppress_popup_front(Lister *lister);", lister_h)
        self.assertIn("BOOL lister_dual_consume_popup_front(Lister *lister);", lister_h)
        self.assertIn("static void lister_dual_draw_command_button", lister_dual_c)
        self.assertIn("DUALF_SUPPRESS_POPUP_FRONT", lister_dual_c)
        self.assertRegex(
            lister_dual_c,
            r"(?s)static void lister_dual_layout.*"
            r"state->command_area\[panel\]\.box\.Width = DUAL_TITLE_BUTTON_WIDTH;.*"
            r"state->command_area\[panel\]\.box\.Left = state->close_area\[panel\]\.box\.Left - "
            r"state->command_area\[panel\]\.box\.Width - 2;.*"
            r"state->name_area\[panel\]\.box\.Width = state->command_area\[panel\]\.box\.Left - "
            r"state->name_area\[panel\]\.box\.Left - 2;",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)static void lister_dual_draw_header.*"
            r"lister_show_name\(lister\);.*"
            r"select_show_info\(lister, 0\);.*"
            r"lister_dual_draw_command_button\(lister, state, side\);.*"
            r"lister_dual_draw_close_button\(lister, state, side\);",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)void lister_dual_activate_at.*"
            r"point_in_element\(&state->command_area\[side\], x, y\)",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)BOOL lister_dual_popup_at\(Lister \*lister, UWORD x, UWORD y\).*"
            r"point_in_element\(&state->command_area\[side\], x, y\)",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)void lister_dual_suppress_popup_front\(Lister \*lister\).*"
            r"state->flags \|= DUALF_SUPPRESS_POPUP_FRONT;",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)BOOL lister_dual_consume_popup_front\(Lister \*lister\).*"
            r"state->flags &= ~DUALF_SUPPRESS_POPUP_FRONT;",
        )
        self.assertRegex(
            lister_idcmp_c,
            r"(?s)case IDCMP_ACTIVEWINDOW:.*"
            r"suppress_front = lister_dual_consume_popup_front\(lister\);.*"
            r"if \(lister_dual_is_side\(lister\) && !lister_dual_one_window\(lister\) &&\s*"
            r"!suppress_front && !lister_dual_popup_at\(lister, msg->MouseX, msg->MouseY\)\)\s*"
            r"lister_dual_to_front\(lister\);",
        )
        self.assertRegex(
            lister_idcmp_c,
            r"(?s)case IDCMP_MOUSEBUTTONS:.*"
            r"lister_dual_is_side\(lister\) && !lister_dual_one_window\(lister\) &&\s*"
            r"!lister_dual_popup_at\(lister, msg->MouseX, msg->MouseY\)\)\s*"
            r"lister_dual_to_front\(lister\);",
        )
        self.assertNotRegex(
            lister_idcmp_c,
            r"point_in_element\(&lister->command_area, msg->MouseX, msg->MouseY\) &&\s*"
            r"!lister_dual_one_window\(lister\)",
        )
        self.assertRegex(
            lister_idcmp_c,
            r"(?s)// Command button\? \(Lister menu\).*"
            r"lister_dual_suppress_popup_front\(lister\);\s*"
            r"function_launch_quick\(FUNCTION_RUN_FUNCTION, function, lister_dual_active_side\(lister\)\);",
        )

    def test_toolbar_activation_does_not_steal_dual_active_side(self):
        lister_h = read_program_source("lister.h")
        lister_dual_c = read_program_source_if_exists("lister_dual.c")
        lister_idcmp_c = read_program_source("lister_idcmp.c")
        lister_proc_c = read_program_source("lister_proc.c")

        self.assertIn("void lister_dual_to_front(Lister *lister);", lister_h)
        self.assertRegex(
            lister_dual_c,
            r"(?s)void lister_dual_to_front\(Lister \*lister\).*"
            r"WindowToFront\(lister->window\);",
        )

        self.assertRegex(
            lister_idcmp_c,
            r"(?s)case IDCMP_ACTIVEWINDOW:.*"
            r"suppress_front = lister_dual_consume_popup_front\(lister\);.*"
            r"if \(lister_dual_is_side\(lister\) && !lister_dual_one_window\(lister\) &&\s*"
            r"!suppress_front && !lister_dual_popup_at\(lister, msg->MouseX, msg->MouseY\)\)\s*"
            r"lister_dual_to_front\(lister\);.*"
            r"if \(!lister_dual_popup_at\(lister, msg->MouseX, msg->MouseY\) &&\s*"
            r"\(\!\(lister->flags & LISTERF_TOOLBAR\) \|\|\s*"
            r"!point_in_element\(&lister->toolbar_area, msg->MouseX, msg->MouseY\)\)\)\s*"
            r"lister_dual_activate\(lister\);",
        )
        self.assertRegex(
            lister_idcmp_c,
            r"(?s)// F10 means window to front.*lister_dual_to_front\(lister\);",
        )
        self.assertRegex(
            lister_idcmp_c,
            r"(?s)case IDCMP_MOUSEBUTTONS:.*"
            r"if \(msg->Qualifier != 0xffff && lister_dual_is_side\(lister\) && !lister_dual_one_window\(lister\) &&.*"
            r"!lister_dual_popup_at\(lister, msg->MouseX, msg->MouseY\)\).*"
            r"lister_dual_to_front\(lister\);.*"
            r"lister_toolbar_tooltip_hide\(lister\);",
        )
        self.assertRegex(
            lister_proc_c,
            r"(?s)case IPC_ACTIVATE:.*ActivateWindow\(lister->window\);.*"
            r"lister_dual_to_front\(lister\);",
        )

    def test_direct_close_restores_remaining_dual_side_as_single_lister(self):
        lister_h = read_program_source("lister.h")
        lister_dual_c = read_program_source_if_exists("lister_dual.c")
        lister_proc_c = read_program_source("lister_proc.c")

        self.assertIn("void lister_dual_detach_closing(Lister *lister);", lister_h)
        self.assertIn("lister_dual_detach_closing(lister);", lister_proc_c)
        self.assertRegex(
            lister_dual_c,
            r"(?s)void lister_dual_detach_closing\(Lister \*lister\).*"
            r"lister_dual_detach\(lister\);",
        )

    def test_dual_lister_display_gadgets_are_rebuilt_after_window_reopen(self):
        lister_h = read_program_source("lister.h")
        lister_window_c = read_program_source("lister_window.c")
        lister_dual_c = read_program_source_if_exists("lister_dual.c")

        self.assertIn("void lister_dual_close_display(Lister *lister);", lister_h)
        self.assertRegex(
            lister_dual_c,
            r"(?s)void lister_dual_close_display\(Lister \*lister\).*"
            r"ListerDualState \*state;.*"
            r"state = lister_dual_get_state\(lister\).*"
            r"state->flags & DUALF_ONE_WINDOW.*"
            r"lister_dual_close_pathfields\(lister, state\);.*"
            r"lister_dual_close_scrollbars\(lister, state\);",
        )
        close_display_func = re.search(
            r"(?s)void lister_dual_close_display\(Lister \*lister\).*?"
            r"\n}\n\nstatic void lister_dual_close_side",
            lister_dual_c,
        ).group(0)
        self.assertNotIn("lister_dual_restore_pathfield", close_display_func)
        self.assertNotIn("lister_dual_remove_scrollbars", close_display_func)
        self.assertRegex(
            lister_dual_c,
            r"(?s)static void lister_dual_close_pathfields.*"
            r"RemoveGList\(lister->window, state->path_field\[LISTER_DUAL_RIGHT\], 1\);.*"
            r"DisposeObject\(\(Object \*\)state->path_field\[LISTER_DUAL_RIGHT\]\);.*"
            r"RemoveGList\(lister->window, lister->path_field, 1\);",
        )
        close_pathfields_func = re.search(
            r"(?s)\nstatic void lister_dual_close_pathfields\(Lister \*lister, ListerDualState \*state\)\n\{.*?"
            r"\n}\n\nstatic BOOL lister_dual_ensure_pathfields",
            lister_dual_c,
        ).group(0)
        self.assertNotIn("AddGList", close_pathfields_func)
        self.assertRegex(
            lister_dual_c,
            r"(?s)static void lister_dual_close_scrollbars.*"
            r"RemoveGList\(lister->window, state->vert_scroller\[side\], 6\);.*"
            r"DisposeObject\(\(Object \*\)state->vert_scroller\[side\]\);.*"
            r"state->flags &= ~DUALF_SCROLLBARS_REMOVED;",
        )
        close_scrollbars_func = re.search(
            r"(?s)\nstatic void lister_dual_close_scrollbars\(Lister \*lister, ListerDualState \*state\)\n\{.*?"
            r"\n}\n\nstatic BOOL lister_dual_ensure_scrollbars",
            lister_dual_c,
        ).group(0)
        self.assertNotIn("AddGList", close_scrollbars_func)
        self.assertRegex(
            lister_window_c,
            r"(?s)void lister_close_window\(Lister \*lister, BOOL run_script\).*"
            r"lister_dual_close_display\(lister\);.*"
            r"CloseWindow\(lister->window\);",
        )

    def test_dual_lister_refreshes_frame_when_screen_metrics_change(self):
        lister_h = read_program_source("lister.h")
        lister_dual_c = read_program_source_if_exists("lister_dual.c")
        lister_display_c = read_program_source("lister_display.c")

        self.assertIn("#define DUALF_FRAME_REFRESH", lister_dual_c)
        self.assertIn("void lister_dual_finish_frame_refresh(Lister *lister);", lister_h)
        self.assertIn("short frame_border_left;", lister_dual_c)
        self.assertIn("short frame_border_top;", lister_dual_c)
        self.assertIn("short frame_border_right;", lister_dual_c)
        self.assertIn("short frame_border_bottom;", lister_dual_c)
        self.assertIn("APTR frame_draw_info;", lister_dual_c)
        self.assertRegex(
            lister_dual_c,
            r"(?s)static BOOL lister_dual_frame_metrics_changed\(Lister \*lister, ListerDualState \*state\).*"
            r"state->frame_border_left != lister->window->BorderLeft.*"
            r"state->frame_border_top != lister->window->BorderTop.*"
            r"state->frame_border_right != lister->window->BorderRight.*"
            r"state->frame_border_bottom != lister->window->BorderBottom.*"
            r"state->frame_draw_info != \(APTR\)GUI->draw_info",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)static void lister_dual_update_parent_button_metrics\(Lister \*lister\).*"
            r"lister->parent_button.TopEdge = lister->window->BorderTop;.*"
            r"lister->parent_button.Width = lister->window->BorderLeft;.*"
            r"lister->parent_button.Height = -\(lister->window->BorderTop \+ lister->window->BorderBottom\);",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)static void lister_dual_prepare_frame_refresh\(Lister \*lister, ListerDualState \*state\).*"
            r"lister_dual_frame_metrics_changed\(lister, state\).*"
            r"lister_dual_update_parent_button_metrics\(lister\);.*"
            r"state->flags \|= DUALF_FRAME_REFRESH;.*"
            r"lister_dual_store_frame_metrics\(lister, state\);",
        )
        prepare_func = re.search(
            r"(?s)\nstatic void lister_dual_prepare_frame_refresh\(Lister \*lister, ListerDualState \*state\)\n\{.*?"
            r"\n}\n\nvoid lister_dual_finish_frame_refresh",
            lister_dual_c,
        ).group(0)
        self.assertNotIn("RefreshWindowFrame(lister->window);", prepare_func)
        self.assertRegex(
            lister_dual_c,
            r"(?s)void lister_dual_finish_frame_refresh\(Lister \*lister\).*"
            r"state->flags & DUALF_FRAME_REFRESH.*"
            r"lister->window->WLayer->Flags & LAYERREFRESH.*"
            r"state->flags &= ~DUALF_FRAME_REFRESH;.*"
            r"RefreshWindowFrame\(lister->window\);",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)BOOL lister_dual_init_display\(Lister \*lister\).*"
            r"lister_dual_prepare_frame_refresh\(lister, state\);.*"
            r"lister_dual_prepare_resize_refresh\(lister\);",
        )
        self.assertRegex(
            lister_dual_c,
            r"(?s)void lister_dual_refresh\(Lister \*lister, unsigned short mode\).*"
            r"if \(mode & LREFRESH_REFRESH\)\s*EndRefresh\(lister->window, TRUE\);.*"
            r"lister_dual_finish_frame_refresh\(lister\);",
        )
        self.assertRegex(
            lister_display_c,
            r"(?s)EndRefresh\(lister->window, TRUE\);\s*"
            r"lister_dual_finish_frame_refresh\(lister\);",
        )


if __name__ == "__main__":
    unittest.main()
