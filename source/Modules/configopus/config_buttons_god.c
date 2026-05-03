#include "config_lib.h"
#include "config_buttons.h"

// Add a row of buttons
BOOL _config_buttons_rows_add(config_buttons_data *data, short insert_row)
{
	struct List templist;
	short col;
	Cfg_Button *button;
	BOOL ok = 1;

	// Lock bank
	GetSemaphore(&data->bank_node->bank->lock, SEMF_EXCLUSIVE, 0);

	// Create some new buttons
	if (!(_config_buttons_create_new(data, &templist, data->bank_node->bank->window.columns)))
	{
		// Failed
		ok = 0;
	}

	// Link to bank's button list
	else
	{
		Cfg_Button *insert_button;

		// Insert at end?
		if (insert_row == -1)
			insert_button = (Cfg_Button *)data->bank_node->bank->buttons.lh_TailPred;

		// Insert at start?
		else if (insert_row == 0)
			insert_button = 0;

		// Get button to insert after
		else
		{
			short num;

			num = (data->bank_node->bank->window.columns * insert_row) - 1;
			for (insert_button = (Cfg_Button *)data->bank_node->bank->buttons.lh_Head, col = 0;
				 insert_button->node.ln_Succ && col < num;
				 insert_button = (Cfg_Button *)insert_button->node.ln_Succ, col++)
				;
		}

		// Go through temporary list until it's empty
		while ((button = (Cfg_Button *)templist.lh_Head)->node.ln_Succ)
		{
			// Remove from temporary list
			Remove(&button->node);

			// Insert after button
			Insert(&data->bank_node->bank->buttons, &button->node, (struct Node *)insert_button);
			insert_button = button;
		}

		// Increment row count
		++data->bank_node->bank->window.rows;
	}

	// Unlock bank
	FreeSemaphore(&data->bank_node->bank->lock);

	// Send refresh
	if (ok)
		_config_buttons_reset(data);
	return ok;
}

// Add a column of buttons
BOOL _config_buttons_columns_add(config_buttons_data *data, short insert_col)
{
	struct List templist;
	BOOL ok = 1;

	// Lock bank
	GetSemaphore(&data->bank_node->bank->lock, SEMF_EXCLUSIVE, 0);

	// Create some new buttons
	if (!(_config_buttons_create_new(data, &templist, data->bank_node->bank->window.rows)))
	{
		// Failed
		ok = 0;
	}

	// Link to bank's button list
	else
	{
		Cfg_Button *insert_button, *next_button, *pos_button, *next_pos, *prev_button = 0;
		short row_pos;

		// Get start of new button list
		insert_button = (Cfg_Button *)templist.lh_Head;
		next_button = (Cfg_Button *)insert_button->node.ln_Succ;

		// Go through button list
		for (pos_button = (Cfg_Button *)data->bank_node->bank->buttons.lh_Head, row_pos = 0; pos_button->node.ln_Succ;)
		{
			// Store next button
			next_pos = (Cfg_Button *)pos_button->node.ln_Succ;

			// Insert at this position?
			if (row_pos == insert_col)
			{
				// Remove button from temp list, insert in button list
				Remove(&insert_button->node);
				Insert(&data->bank_node->bank->buttons, &insert_button->node, (struct Node *)prev_button);
				insert_button = 0;
			}

			// Store this button pointer
			prev_button = pos_button;

			// Increment row position
			++row_pos;

			// End of a row?
			if ((row_pos % data->bank_node->bank->window.columns) == 0)
			{
				// Adding to end of row?
				if (insert_col == -1)
				{
					// Remove button from temp list, insert in button list
					Remove(&insert_button->node);
					Insert(&data->bank_node->bank->buttons, &insert_button->node, (struct Node *)pos_button);
					insert_button = 0;
				}

				// Reset row position
				row_pos = 0;
			}

			// Need to get a new button from temp list?
			if (!insert_button)
			{
				if (!next_button || !next_button->node.ln_Succ)
					break;
				insert_button = next_button;
				next_button = (Cfg_Button *)next_button->node.ln_Succ;
			}

			// Get next button
			pos_button = next_pos;
		}

		// Increment column count
		++data->bank_node->bank->window.columns;
	}

	// Unlock bank
	FreeSemaphore(&data->bank_node->bank->lock);

	// Send refresh
	if (ok)
		_config_buttons_reset(data);
	return ok;
}

/*****************************************************************************/

// Remove a row
BOOL _config_buttons_rows_remove(config_buttons_data *data)
{
	BOOL ok = 0;

	// Lock bank
	GetSemaphore(&data->bank_node->bank->lock, SEMF_EXCLUSIVE, 0);

	// Check there's more than one row
	if (data->bank_node->bank->window.rows > 1)
	{
		short old_count;
		short new_rows;

		// Get the total number of buttons
		old_count = data->bank_node->bank->window.columns * data->bank_node->bank->window.rows;

		// Find the next factor for the row count
		for (new_rows = data->bank_node->bank->window.rows - 1; new_rows > 0; new_rows--)
		{
			// Is this a factor?
			if ((old_count % new_rows) == 0)
				break;
		}

		// Has the number changed?
		if (new_rows != data->bank_node->bank->window.rows)
		{
			// Store the new number of rows
			data->bank_node->bank->window.rows = new_rows;

			// Calculate the new number of columns
			data->bank_node->bank->window.columns = old_count / new_rows;
			ok = 1;
		}
	}

	// Unlock bank
	FreeSemaphore(&data->bank_node->bank->lock);

	// Send refresh
	if (ok)
		_config_buttons_reset(data);
	return ok;
}

// Remove a column
BOOL _config_buttons_columns_remove(config_buttons_data *data)
{
	BOOL ok = 0;

	// Lock bank
	GetSemaphore(&data->bank_node->bank->lock, SEMF_EXCLUSIVE, 0);

	// Check there's more than one column
	if (data->bank_node->bank->window.columns > 1)
	{
		short old_count;
		short new_cols;

		// Get the total number of buttons
		old_count = data->bank_node->bank->window.columns * data->bank_node->bank->window.rows;

		// Find the next factor for the column count
		for (new_cols = data->bank_node->bank->window.columns - 1; new_cols > 0; new_cols--)
		{
			// Is this a factor?
			if ((old_count % new_cols) == 0)
				break;
		}

		// Has the number changed?
		if (new_cols != data->bank_node->bank->window.columns)
		{
			// Store the new number of columns
			data->bank_node->bank->window.columns = new_cols;

			// Calculate the new number of rows
			data->bank_node->bank->window.rows = old_count / new_cols;
			ok = 1;
		}
	}

	// Unlock bank
	FreeSemaphore(&data->bank_node->bank->lock);

	// Send refresh
	if (ok)
		_config_buttons_reset(data);
	return ok;
}

/*****************************************************************************/

// Remove some rows
void _config_buttons_remove_rows(config_buttons_data *data, short pos, short count)
{
	struct List templist;
	short col_count = 0, row_count = 0, rem_count = 0;
	Cfg_Button *button, *next;

	// Initialise temporary list
	NewList(&templist);

	// Go through buttons
	for (button = (Cfg_Button *)data->bank_node->bank->buttons.lh_Head; button->node.ln_Succ;)
	{
		// Store next button
		next = (Cfg_Button *)button->node.ln_Succ;

		// Can we remove this button?
		if (row_count >= pos && rem_count < count)
		{
			// Remove from list, add to temp list
			Remove(&button->node);
			AddTail(&templist, &button->node);
		}

		// Increment column count
		++col_count;

		// End of a row?
		if (col_count == data->bank_node->bank->window.columns)
		{
			col_count = 0;
			if (row_count >= pos)
			{
				++rem_count;
				if (rem_count >= count)
					break;
			}
			++row_count;
		}

		// Get next button
		button = next;
	}

	// Free temporary list
	FreeButtonList(&templist);

	// Decrement row count
	data->bank_node->bank->window.rows -= count;
}

// Remove some columns
void _config_buttons_remove_columns(config_buttons_data *data, short pos, short count)
{
	struct List templist;
	short col_count = 0, rem_count = 0;
	Cfg_Button *button, *next;

	// Initialise temporary list
	NewList(&templist);

	// Go through buttons
	for (button = (Cfg_Button *)data->bank_node->bank->buttons.lh_Head; button->node.ln_Succ;)
	{
		// Store next button
		next = (Cfg_Button *)button->node.ln_Succ;

		// Can we remove this button?
		if (col_count >= pos && rem_count < count)
		{
			// Remove from list, add to temp list
			Remove(&button->node);
			AddTail(&templist, &button->node);
			++rem_count;
		}

		// Increment column count
		++col_count;

		// End of a row?
		if (col_count == data->bank_node->bank->window.columns)
		{
			col_count = 0;
			rem_count = 0;
		}

		// Get next button
		button = next;
	}

	// Free temporary list
	FreeButtonList(&templist);

	// Decrement column count
	data->bank_node->bank->window.columns -= count;
}

// Handles key press in edit mode
BOOL _config_buttons_handle_key(config_buttons_data *data, UWORD code, UWORD qual)
{
	Cfg_Button *button;
	Cfg_Button *redraw_1 = 0, *redraw_2 = 0;
	BOOL redraw = 0;
	short sel_col, sel_row;
	BOOL change = 0;

	// Get current selection
	if ((_config_buttons_get_colrow(data, &sel_col, &sel_row)) == -1)
		return 0;

	// Get selected button
	if (!(button = _config_buttons_get_button(data, sel_col, sel_row)))
		return 0;

	// Look at key
	switch (code)
	{
	case CURSORLEFT:
	case CURSORRIGHT:

		// Check this isn't the first or last button
		if ((code == CURSORLEFT && sel_col > 0) ||
			(code == CURSORRIGHT && sel_col < data->bank_node->bank->window.columns - 1))
		{
			short delta;

			// Turn flash off
			_config_buttons_flash(data, 0);

			// Get delta
			if (code == CURSORLEFT)
				delta = -1;
			else
				delta = 1;

			// Change selection pointer
			sel_col += delta;

			// If shift is down, move button
			if (qual & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
			{
				// If alt isn't down, move button left or right
				if (!(qual & (IEQUALIFIER_LALT | IEQUALIFIER_RALT)))
				{
					struct Node *swap, *after;

					// Get the button to swap with
					if (code == CURSORLEFT)
					{
						swap = button->node.ln_Pred;
						after = swap->ln_Pred;
					}
					else
					{
						swap = button->node.ln_Succ;
						after = swap;
					}

					// Remove this button
					Remove((struct Node *)button);

					// Insert before swap button
					Insert(&data->bank_node->bank->buttons, (struct Node *)button, after);

					// Set redraw pointers
					redraw_1 = button;
					redraw_2 = (Cfg_Button *)swap;

					// Set change flag
					change = 1;
				}

				// Otherwise, move the whole column
				else if (data->bank_node->bank->window.columns > 1)
				{
					short row;

					// Go through the rows
					for (row = 0; row < data->bank_node->bank->window.rows; row++)
					{
						struct Node *swap, *after;

						// Get button in this row
						if (!(button = _config_buttons_get_button(data, sel_col - delta, row)))
							continue;

						// Get the button to swap with
						if (code == CURSORLEFT)
						{
							swap = button->node.ln_Pred;
							after = swap->ln_Pred;
						}
						else
						{
							swap = button->node.ln_Succ;
							after = swap;
						}

						// Remove this button
						Remove((struct Node *)button);

						// Insert before swap button
						Insert(&data->bank_node->bank->buttons, (struct Node *)button, after);

						// If full refresh is not set, redraw these buttons
						if (!redraw)
						{
							_config_buttons_redraw_button(data, data->bank_node, button);
							_config_buttons_redraw_button(data, data->bank_node, (Cfg_Button *)swap);
						}

						// Set change flag
						change = 1;
					}
				}
			}
		}
		break;

	case CURSORUP:
	case CURSORDOWN:

		// Check this isn't the first or last row
		if ((code == CURSORUP && sel_row > 0) ||
			(code == CURSORDOWN && sel_row < data->bank_node->bank->window.rows - 1))
		{
			short delta;

			// Turn flash off
			_config_buttons_flash(data, 0);

			// Get delta
			if (code == CURSORUP)
				delta = -1;
			else
				delta = 1;

			// If shift is down, move button
			if (qual & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
			{
				// If alt isn't down, move button up or down
				if (!(qual & (IEQUALIFIER_LALT | IEQUALIFIER_RALT)))
				{
					Cfg_Button *swap;
					struct Node *temp;
					short count;

					// Get the button to swap with
					for (swap = button, count = 0;
						 swap->node.ln_Succ && swap->node.ln_Pred && count < data->bank_node->bank->window.columns;
						 swap = (Cfg_Button *)((delta == -1) ? swap->node.ln_Pred : swap->node.ln_Succ), count++)
						;

					// Valid button?
					if (!swap->node.ln_Succ || !swap->node.ln_Pred)
						break;

					// Remove the two buttons
					Remove((struct Node *)button);
					Remove((struct Node *)swap);

					// Is there only one column?
					if (data->bank_node->bank->window.columns == 1)
					{
						// Old above the new?
						if (button->node.ln_Succ == &swap->node)
						{
							Insert(&data->bank_node->bank->buttons, (struct Node *)swap, button->node.ln_Pred);
							Insert(&data->bank_node->bank->buttons, (struct Node *)button, (struct Node *)swap);
						}

						// New above the old
						else
						{
							Insert(&data->bank_node->bank->buttons, (struct Node *)button, swap->node.ln_Pred);
							Insert(&data->bank_node->bank->buttons, (struct Node *)swap, (struct Node *)button);
						}
					}

					// More than one column
					else
					{
						// Swap button positions
						temp = button->node.ln_Pred;
						Insert(&data->bank_node->bank->buttons, (struct Node *)button, swap->node.ln_Pred);
						Insert(&data->bank_node->bank->buttons, (struct Node *)swap, temp);
					}

					// Change selection pointer
					sel_row += delta;

					// Set redraw pointers
					redraw_1 = button;
					redraw_2 = swap;

					// Set change flag
					change = 1;
				}

				// Otherwise, move row up or down
				else if (data->bank_node->bank->window.rows > 1)
				{
					struct Node *first, *last;
					struct Node *row_last;

					// Get the first and last buttons in the row
					if (!(first = (struct Node *)_config_buttons_get_button(data, 0, sel_row)) ||
						!(last = (struct Node *)_config_buttons_get_button(
							  data, data->bank_node->bank->window.columns - 1, sel_row)))
						break;

					// Get the last button in the row to insert this after
					row_last = (struct Node *)_config_buttons_get_button(
						data, data->bank_node->bank->window.columns - 1, sel_row + ((code == CURSORUP) ? -2 : 1));

					// Detach from list
					first->ln_Pred->ln_Succ = last->ln_Succ;
					last->ln_Succ->ln_Pred = first->ln_Pred;

					// Link after new row
					if (row_last)
					{
						last->ln_Succ = row_last->ln_Succ;
						row_last->ln_Succ->ln_Pred = last;
						row_last->ln_Succ = first;
						first->ln_Pred = row_last;
					}

					// Link to head of list
					else
					{
						last->ln_Succ = data->bank_node->bank->buttons.lh_Head;
						first->ln_Pred = data->bank_node->bank->buttons.lh_Head->ln_Pred;
						data->bank_node->bank->buttons.lh_Head->ln_Pred = last;
						data->bank_node->bank->buttons.lh_Head = first;
					}

					// Change selection pointer
					sel_row += delta;

					// Set change flag
					change = 1;

					// Set redraw flag
					redraw = 1;
				}
			}

			// Otherwise just change selection pointer
			else
			{
				sel_row += delta;
			}
		}
		break;
	}

	// Redraw buttons?
	if (!redraw)
	{
		if (redraw_1)
			_config_buttons_redraw_button(data, data->bank_node, redraw_1);
		if (redraw_2)
			_config_buttons_redraw_button(data, data->bank_node, redraw_2);
	}

	// Store new row/column
	data->select_col = sel_col;
	data->select_row = sel_row;

	// Send new row/column
	if (IPC_Command(data->bank_node->button_ipc, BUTTONEDIT_SET_SELECTION, sel_col, (APTR)(IPTR)sel_row, 0, REPLY_NO_PORT))
		redraw = 1;

	// Refresh window
	if (redraw)
		_config_buttons_refresh(data, data->bank_node, BUTREFRESH_REFRESH);

	// Turn flash back on
	_config_buttons_flash(data, 1);

	return change;
}

// Delete a row
BOOL _config_buttons_delete_row(config_buttons_data *data, short row)
{
	if (data->bank_node->bank->window.rows > 1)
	{
		_config_buttons_remove_rows(data, row, 1);
		/*
				_config_buttons_refresh(data,data->bank_node,BUTREFRESH_SELECTOR|BUTREFRESH_RESIZE|BUTREFRESH_REFRESH);
				_config_buttons_update(data);
		*/
		_config_buttons_reset(data);
		return 1;
	}
	return 0;
}

// Delete a row
BOOL _config_buttons_delete_column(config_buttons_data *data, short col)
{
	if (data->bank_node->bank->window.columns > 1)
	{
		_config_buttons_remove_columns(data, col, 1);
		/*
				_config_buttons_refresh(data,data->bank_node,BUTREFRESH_SELECTOR|BUTREFRESH_RESIZE|BUTREFRESH_REFRESH);
				_config_buttons_update(data);
		*/
		_config_buttons_reset(data);
		return 1;
	}
	return 0;
}
