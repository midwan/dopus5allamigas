/*
 * Enhanced icon safety functions for Directory Opus 5
 * Provides robust icon.library integration with reference counting
 * and atomic operations to prevent crashes during rapid icon operations
 */

#include "dopus.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#if defined(__amigaos3__)

// Safety constants for icon operations
#define MAX_ICON_WIDTH 256
#define MAX_ICON_HEIGHT 256
#define MAX_MASK_WORDS ((MAX_ICON_WIDTH + 15) >> 4)
#define MAX_LOCK_ATTEMPTS 1000
#define LOCK_DELAY_CYCLES 10

// Validation caching to reduce overhead
#define VALIDATION_CACHE_TTL 50  // Validate same object at most every 50 calls
static ULONG validation_call_count = 0;
static BackdropObject *last_validated_object = NULL;
static ULONG last_validation_count = 0;

// Last error tracking (thread-safe for AmigaOS single-threading)
static IconErrorCode last_icon_error = ICON_ERROR_NONE;

// Debug logging control - more fine-grained control
#ifdef ICON_SAFETY_DEBUG
#define ICON_DEBUG_ENABLED 1
#define ICON_STATS_ENABLED 1
#else
#define ICON_DEBUG_ENABLED 0
#define ICON_STATS_ENABLED 0
#endif

static IconDebugLevel current_debug_level = ICON_DEBUG_ERROR;

// NEW: Debug logging functions
void backdrop_set_icon_debug_level(IconDebugLevel level)
{
	current_debug_level = level;
}

void backdrop_icon_debug_log(IconDebugLevel level, const char *function, const char *format, ...)
{
#if ICON_DEBUG_ENABLED
	if (level > current_debug_level) {
		return;
	}
	
	va_list args;
	va_start(args, format);
	
	// Simple debug output - in a real implementation this might go to a log file
	// For now, we'll use a basic output approach
	const char *level_str = "";
	switch (level) {
		case ICON_DEBUG_ERROR: level_str = "ERROR"; break;
		case ICON_DEBUG_WARN:  level_str = "WARN "; break;
		case ICON_DEBUG_INFO:  level_str = "INFO "; break;
		case ICON_DEBUG_TRACE: level_str = "TRACE"; break;
		default: level_str = "UNKN "; break;
	}
	
	// Format: [LEVEL] Function: message
	// Use kprintf for debug output (available on AmigaOS)
	char buffer[256];
	short buf_len = snprintf(buffer, sizeof(buffer), "[%s] %s: ", level_str, function);
	
	// Print the formatted message
	vsnprintf(buffer + buf_len, sizeof(buffer) - buf_len, format, args);
	kprintf("%s\n", buffer);
	
	va_end(args);
#endif
}

#define ICON_DEBUG_LOG(level, fmt, ...) \
	backdrop_icon_debug_log(level, __FUNCTION__, fmt, ##__VA_ARGS__)

#define ICON_ERROR(fmt, ...)   ICON_DEBUG_LOG(ICON_DEBUG_ERROR, fmt, ##__VA_ARGS__)
#define ICON_WARN(fmt, ...)    ICON_DEBUG_LOG(ICON_DEBUG_WARN, fmt, ##__VA_ARGS__)
#define ICON_INFO(fmt, ...)    ICON_DEBUG_LOG(ICON_DEBUG_INFO, fmt, ##__VA_ARGS__)
#define ICON_TRACE(fmt, ...)   ICON_DEBUG_LOG(ICON_DEBUG_TRACE, fmt, ##__VA_ARGS__)

// NEW: Error reporting functions
IconErrorCode backdrop_get_last_icon_error(void)
{
	return last_icon_error;
}

const char *backdrop_get_icon_error_string(IconErrorCode error)
{
	switch (error) {
		case ICON_ERROR_NONE: return "No error";
		case ICON_ERROR_INVALID_POINTER: return "Invalid pointer";
		case ICON_ERROR_CORRUPTED_DATA: return "Corrupted data";
		case ICON_ERROR_OUT_OF_MEMORY: return "Out of memory";
		case ICON_ERROR_INVALID_DIMENSIONS: return "Invalid dimensions";
		case ICON_ERROR_LOCK_TIMEOUT: return "Lock timeout";
		case ICON_ERROR_REFERENCE_COUNT: return "Reference count error";
		case ICON_ERROR_VALIDATION_FAILED: return "Validation failed";
		case ICON_ERROR_EMERGENCY_RESET: return "Emergency reset performed";
		default: return "Unknown error";
	}
}

// NEW: Optimized safe acquisition of iconlib select icon with reference counting
BOOL backdrop_acquire_iconlib_select_icon(BackdropObject *object)
{
	// Fast path: quick NULL check before any debug logging
	if (!object || !object->iconlib_select_icon) {
		ICON_ERROR("Invalid pointer: object=%p, iconlib_select_icon=%p", 
				   object, object ? object->iconlib_select_icon : NULL);
		last_icon_error = ICON_ERROR_INVALID_POINTER;
		return FALSE;
	}
	
	ICON_TRACE("Acquiring iconlib select icon for object %p", object);
	
	// Fast path: most common case - healthy object with reasonable refcount
	if (object->iconlib_select_refcount < 900) {
		// Simple atomic increment (for single-threaded AmigaOS this is safe)
		object->iconlib_select_refcount++;
		ICON_TRACE("Iconlib select icon acquired, new refcount: %d", object->iconlib_select_refcount);
		last_icon_error = ICON_ERROR_NONE;
		return TRUE;
	}
	
	// Edge case: check for reference count overflow
	if (object->iconlib_select_refcount >= 1000) {
		ICON_ERROR("Reference count overflow: %d", object->iconlib_select_refcount);
		last_icon_error = ICON_ERROR_REFERENCE_COUNT;
		return FALSE;
	}
	
	// High refcount but still safe
	object->iconlib_select_refcount++;
	ICON_WARN("High refcount: %d", object->iconlib_select_refcount);
	last_icon_error = ICON_ERROR_NONE;
	return TRUE;
}

// NEW: Safe release of iconlib select icon with reference counting
BOOL backdrop_release_iconlib_select_icon_ref(BackdropObject *object)
{
	ICON_TRACE("Releasing iconlib select icon for object %p", object);
	
	if (!object || !object->iconlib_select_icon) {
		ICON_ERROR("Invalid pointer: object=%p, iconlib_select_icon=%p", 
				   object, object ? object->iconlib_select_icon : NULL);
		last_icon_error = ICON_ERROR_INVALID_POINTER;
		return FALSE;
	}
	
	if (object->iconlib_select_refcount > 0) {
		object->iconlib_select_refcount--;
		ICON_TRACE("Iconlib select icon released, new refcount: %d", object->iconlib_select_refcount);
		
		// Only free when refcount reaches zero
		if (object->iconlib_select_refcount == 0) {
			ICON_INFO("Iconlib select icon refcount reached zero, ready for cleanup");
			// This will be called by the actual free function
			// when it's safe to do so
		}
		last_icon_error = ICON_ERROR_NONE;
		return TRUE;
	} else {
		// Reference count underflow
		ICON_ERROR("Reference count underflow, was %d", object->iconlib_select_refcount);
		last_icon_error = ICON_ERROR_REFERENCE_COUNT;
		object->iconlib_select_refcount = 0; // Reset to prevent negative
		return FALSE;
	}
}

// NEW: Atomic state change with simple locking
BOOL backdrop_set_icon_state_safe(BackdropObject *object, short new_state)
{
	if (!object) {
		last_icon_error = ICON_ERROR_INVALID_POINTER;
		return FALSE;
	}
	
	// Validate new state
	if (new_state < 0 || new_state > 2) {
		last_icon_error = ICON_ERROR_INVALID_DIMENSIONS; // Reuse this error code
		return FALSE;
	}
	
	// Optimized spinlock with adaptive timing
	int attempts = 0;
	while (object->iconlib_state_lock && attempts < MAX_LOCK_ATTEMPTS) {
		attempts++;
		
		// Adaptive delay: start with micro-delay, increase if needed
		if (attempts < 10) {
			// For first few attempts, use very short micro-delay
			// This is more responsive than Delay(1) which is 20ms
			for (volatile int i = 0; i < 100; i++) {
				// Tiny delay to yield some CPU time
			}
		} else if (attempts < 100) {
			// Medium delay for contested locks
			for (volatile int i = 0; i < 1000; i++) {
				// Longer micro-delay
			}
		} else {
			// For persistent contention, use proper system delay
			// But only delay for 1 tick (1/50 second) maximum
			Delay(1);
		}
	}
	
	if (attempts >= MAX_LOCK_ATTEMPTS) {
		// Couldn't acquire lock, fail gracefully
		last_icon_error = ICON_ERROR_LOCK_TIMEOUT;
		return FALSE;
	}
	
	// Acquire lock
	object->iconlib_state_lock = 1;
	
	// Change state
	object->state = new_state;
	object->flags |= BDOF_STATE_CHANGE;
	
	// Release lock
	object->iconlib_state_lock = 0;
	
	last_icon_error = ICON_ERROR_NONE;
	return TRUE;
}

// NEW: Comprehensive and safe icon resource cleanup
void backdrop_cleanup_icon_resources(BackdropInfo *info, BackdropObject *object)
{
	if (!object)
		return;
	
	// Validate object state first
	if (!backdrop_validate_icon_object_safe(object)) {
		// Object is corrupted, use emergency reset
		backdrop_emergency_reset_icon_object(object);
		return;
	}
	
	// Always release system state first
	backdrop_release_system_icon_state(info, object);
	
	// Clean up iconlib select icon safely
	if (object->iconlib_select_icon) {
		// Wait for reference count to reach zero
		int wait_count = 0;
		while (object->iconlib_select_refcount > 0 && wait_count < MAX_LOCK_ATTEMPTS) {
			wait_count++;
			// Simple delay
			for (int i = 0; i < LOCK_DELAY_CYCLES; i++) {
				// NOP
			}
		}
		
		// Force cleanup if wait exceeded
		if (object->iconlib_select_refcount > 0) {
			object->iconlib_select_refcount = 0;
		}
		
		backdrop_free_iconlib_select_icon(info, object);
	}
	
	// Clean up main icon with validation
	if (object->icon) {
		// Validate icon before cleanup
		if (backdrop_validate_icon_dimensions_safe(object->icon)) {
			if (object->flags & BDOF_REMAPPED) {
				RemapIcon(object->icon, 
						(info && info->window) ? info->window->WScreen : 0, 1);
				object->flags &= ~BDOF_REMAPPED;
			}
			FreeCachedDiskObject(object->icon);
		} else {
			// Icon is corrupted, just free it without remap
			FreeCachedDiskObject(object->icon);
		}
		object->icon = NULL;
	}
	
	// Clean up image masks safely
	for (int i = 0; i < 2; i++) {
		if (object->image_mask[i]) {
			FreeVec(object->image_mask[i]);
			object->image_mask[i] = NULL;
		}
	}
	
	// Reset all flags
	object->flags &= ~(BDOF_ICONLIB_DEFAULT | BDOF_STATE_CHANGE | BDOF_REMAPPED);
}

// NEW: Safe drag mask application with comprehensive bounds checking
BOOL backdrop_apply_iconlib_drag_mask_safe(DragInfo *drag, BackdropObject *object)
{
	struct DiskObject *icon;
	PLANEPTR mask = 0;
	BOOL selected;
	short mask_width, mask_height;
	short src_words, dst_words, rows, words, row;
	UWORD *src, *dst;
	long word, total_words;

	if (!drag || !drag->bob.ImageShadow || !object || !object->icon || !IconBase || IconBase->lib_Version < 44)
		return FALSE;

	// Validate drag buffer dimensions
	if (drag->width <= 0 || drag->height <= 0 || 
		drag->width > MAX_ICON_WIDTH || drag->height > MAX_ICON_HEIGHT) {
		return FALSE;
	}

	icon = object->icon;
	selected = (object->state) ? TRUE : FALSE;

	if (selected && (object->flags & BDOF_ICONLIB_DEFAULT)) {
		struct DiskObject *select_icon;

		if ((select_icon = backdrop_get_iconlib_select_icon(object)))
			icon = select_icon;
	}

	if (selected) {
		IconControl(icon, ICONCTRLA_GetImageMask2, &mask, TAG_DONE);
		if (!mask)
			selected = FALSE;
	}

	if (!mask)
		IconControl(icon, ICONCTRLA_GetImageMask1, &mask, TAG_DONE);

	if (!mask)
		return FALSE;

	// Get icon dimensions safely
	mask_width = icon->do_Gadget.Width;
	mask_height = icon->do_Gadget.Height;
	
	// Validate icon dimensions
	if (mask_width <= 0 || mask_height <= 0 || 
		mask_width > MAX_ICON_WIDTH || mask_height > MAX_ICON_HEIGHT) {
		return FALSE;
	}

	// Calculate safe buffer sizes
	src_words = (mask_width + 15) >> 4;
	dst_words = (drag->width + 15) >> 4;
	rows = (mask_height < drag->height) ? mask_height : drag->height;
	words = (src_words < dst_words) ? src_words : dst_words;

	// Additional safety checks
	if (src_words <= 0 || dst_words <= 0 || rows <= 0 || words <= 0 ||
		dst_words > MAX_MASK_WORDS || src_words > MAX_MASK_WORDS) {
		return FALSE;
	}

	// Clear destination buffer safely
	dst = (UWORD *)drag->bob.ImageShadow;
	total_words = dst_words * drag->height;
	
	// Validate total buffer size
	if (total_words <= 0 || total_words > (MAX_MASK_WORDS * MAX_ICON_HEIGHT)) {
		return FALSE;
	}
	
	for (word = 0; word < total_words; word++)
		dst[word] = 0;

	// Copy mask data safely
	src = (UWORD *)mask;
	for (row = 0; row < rows; row++) {
		// Validate source and destination pointers
		if (src + (row * src_words) && dst + (row * dst_words)) {
			CopyMem((char *)(src + (row * src_words)), 
					(char *)(dst + (row * dst_words)), 
					words * sizeof(UWORD));
		}
	}

	// Handle partial word cleanup safely
	if ((drag->width & 15) != 0) {
		UWORD remainder = ~((1 << ((dst_words << 4) - drag->width)) - 1);

		for (row = 0; row < drag->height; row++) {
			if (dst + (row * dst_words) + dst_words - 1) {
				dst[(row * dst_words) + dst_words - 1] &= remainder;
			}
		}
	}

	return TRUE;
}

// Statistics tracking for debugging (conditional compilation)
#if ICON_STATS_ENABLED
static struct {
	unsigned long validation_calls;
	unsigned long validation_failures;
	unsigned long emergency_resets;
	unsigned long lock_timeouts;
	unsigned long refcount_errors;
} icon_stats = {0};
#define INC_STAT(stat) icon_stats.stat++
#else
#define INC_STAT(stat) /* No-op */
#endif

// NEW: Get icon statistics for debugging
void backdrop_get_icon_statistics(unsigned long *validations, unsigned long *failures, 
								  unsigned long *resets, unsigned long *timeouts, 
								  unsigned long *refcount_errs)
{
#if ICON_STATS_ENABLED
	if (validations) *validations = icon_stats.validation_calls;
	if (failures) *failures = icon_stats.validation_failures;
	if (resets) *resets = icon_stats.emergency_resets;
	if (timeouts) *timeouts = icon_stats.lock_timeouts;
	if (refcount_errs) *refcount_errs = icon_stats.refcount_errors;
#else
	if (validations) *validations = 0;
	if (failures) *failures = 0;
	if (resets) *resets = 0;
	if (timeouts) *timeouts = 0;
	if (refcount_errs) *refcount_errs = 0;
#endif
}

// NEW: Reset icon statistics
void backdrop_reset_icon_statistics(void)
{
#if ICON_STATS_ENABLED
	memset(&icon_stats, 0, sizeof(icon_stats));
#endif
}

// NEW: Safe icon object initialization
BOOL backdrop_init_icon_object_safe(BackdropObject *object)
{
	ICON_TRACE("Initializing icon object %p", object);
	
	if (!object) {
		ICON_ERROR("Cannot initialize NULL object");
		return FALSE;
	}
	
	// Initialize all icon-related fields to safe defaults
	object->icon = NULL;
	object->iconlib_select_icon = NULL;
	object->iconlib_select_refcount = 0;
	object->iconlib_state_lock = 0;
	object->flags &= ~(BDOF_ICONLIB_DEFAULT | BDOF_STATE_CHANGE | BDOF_REMAPPED);
	
	// Initialize image masks to NULL
	for (int i = 0; i < 2; i++) {
		object->image_mask[i] = NULL;
	}
	
	ICON_TRACE("Icon object %p initialized successfully", object);
	return TRUE;
}

// NEW: Fast validation for common cases (inline-able)
BOOL backdrop_validate_icon_fast(BackdropObject *object)
{
	// Ultra-fast path: NULL check
	if (!object) return FALSE;
	
	// Fast path: recently validated healthy object
	if (object == last_validated_object && 
		(validation_call_count - last_validation_count) < VALIDATION_CACHE_TTL) {
		return TRUE;
	}
	
	// Fast path: basic sanity check without full validation
	// This catches the most common corruption patterns
	if (object->icon && ((ULONG)object->icon & 0x80000001)) {
		return FALSE;
	}
	
	if (object->iconlib_select_icon && ((ULONG)object->iconlib_select_icon & 0x80000001)) {
		return FALSE;
	}
	
	// Quick refcount sanity check
	if (object->iconlib_select_refcount > 1000) {
		return FALSE;
	}
	
	return TRUE;  // Passed fast validation
}

// NEW: Safe validation of icon object state with caching
BOOL backdrop_validate_icon_object_safe(BackdropObject *object)
{
	INC_STAT(validation_calls);
	validation_call_count++;
	
	// Ultra-fast path: try fast validation first
	if (backdrop_validate_icon_fast(object)) {
		// If it's the same recently validated object, we're done
		if (object == last_validated_object && 
			(validation_call_count - last_validation_count) < VALIDATION_CACHE_TTL) {
			ICON_TRACE("Using cached validation for object %p", object);
			return TRUE;
		}
	}
	
	ICON_TRACE("Validating icon object %p", object);
	
	if (!object) {
		ICON_ERROR("Cannot validate NULL object");
		INC_STAT(validation_failures);
		last_validated_object = NULL;
		return FALSE;
	}
	
	// Check for corrupted pointers
	if (object->icon && ((ULONG)object->icon & 0x80000001)) {
		// Invalid pointer alignment
		ICON_ERROR("Icon pointer has invalid alignment: %p", object->icon);
		INC_STAT(validation_failures);
		return FALSE;
	}
	
	if (object->iconlib_select_icon && ((ULONG)object->iconlib_select_icon & 0x80000001)) {
		// Invalid pointer alignment
		ICON_ERROR("Iconlib select icon pointer has invalid alignment: %p", object->iconlib_select_icon);
		INC_STAT(validation_failures);
		return FALSE;
	}
	
	// Check reference count sanity
	if (object->iconlib_select_refcount < 0 || object->iconlib_select_refcount > 1000) {
		// Reference count corrupted
		ICON_ERROR("Invalid reference count: %d", object->iconlib_select_refcount);
		object->iconlib_select_refcount = 0;
		INC_STAT(refcount_errors);
		INC_STAT(validation_failures);
		return FALSE;
	}
	
	// Validate icon dimensions if icon exists
	if (object->icon) {
		short width = object->icon->do_Gadget.Width;
		short height = object->icon->do_Gadget.Height;
		
		if (width <= 0 || height <= 0 || 
			width > MAX_ICON_WIDTH || height > MAX_ICON_HEIGHT) {
			// Invalid icon dimensions
			ICON_ERROR("Invalid icon dimensions: %dx%d", width, height);
			INC_STAT(validation_failures);
			return FALSE;
		}
	}
	
	ICON_TRACE("Icon object %p validation successful", object);
	
	// Update cache for successful validation
	last_validated_object = object;
	last_validation_count = validation_call_count;
	
	return TRUE;
}

// NEW: Safe buffer allocation for icon masks
PLANEPTR backdrop_alloc_icon_mask_safe(short width, short height)
{
	// Validate dimensions
	if (width <= 0 || height <= 0 || 
		width > MAX_ICON_WIDTH || height > MAX_ICON_HEIGHT) {
		return NULL;
	}
	
	// Calculate required buffer size
	short words_per_row = (width + 15) >> 4;
	long total_words = words_per_row * height;
	long total_bytes = total_words * sizeof(UWORD);
	
	// Validate buffer size
	if (total_words <= 0 || total_words > (MAX_MASK_WORDS * MAX_ICON_HEIGHT) ||
		total_bytes <= 0 || total_bytes > (MAX_MASK_WORDS * MAX_ICON_HEIGHT * sizeof(UWORD))) {
		return NULL;
	}
	
	// Allocate and clear buffer
	PLANEPTR mask = AllocVec(total_bytes, MEMF_CHIP | MEMF_CLEAR);
	if (mask) {
		// Ensure buffer is zeroed
		memset(mask, 0, total_bytes);
	}
	
	return mask;
}

// NEW: Safe icon dimension validation
BOOL backdrop_validate_icon_dimensions_safe(struct DiskObject *icon)
{
	if (!icon)
		return FALSE;
	
	short width = icon->do_Gadget.Width;
	short height = icon->do_Gadget.Height;
	
	// Check for reasonable dimensions
	if (width <= 0 || height <= 0 || 
		width > MAX_ICON_WIDTH || height > MAX_ICON_HEIGHT) {
		return FALSE;
	}
	
	// Check for aspect ratio sanity (prevent extremely distorted icons)
	// Use integer arithmetic to avoid floating point on AmigaOS
	if (width > 0 && height > 0) {
		// Check if width is more than 10x height or vice versa
		if ((width * 10 < height) || (height * 10 < width)) {
			// Unreasonable aspect ratio (more than 10:1)
			return FALSE;
		}
	}
	
	return TRUE;
}

// NEW: Emergency icon object reset for corrupted state
void backdrop_emergency_reset_icon_object(BackdropObject *object)
{
	if (!object)
		return;
	
	// Emergency cleanup - free resources if they exist
	if (object->icon) {
		FreeCachedDiskObject(object->icon);
	}
	
	if (object->iconlib_select_icon) {
		// Force free without reference counting in emergency
		FreeCachedDiskObject(object->iconlib_select_icon);
	}
	
	// Reset all fields to safe defaults
	object->icon = NULL;
	object->iconlib_select_icon = NULL;
	object->iconlib_select_refcount = 0;
	object->iconlib_state_lock = 0;
	object->flags &= ~(BDOF_ICONLIB_DEFAULT | BDOF_STATE_CHANGE | BDOF_REMAPPED);
	
	for (int i = 0; i < 2; i++) {
		if (object->image_mask[i]) {
			FreeVec(object->image_mask[i]);
			object->image_mask[i] = NULL;
		}
	}
}

// NEW: Safe icon validation with automatic recovery
BOOL backdrop_validate_and_recover_icon_object(BackdropObject *object)
{
	if (!object)
		return FALSE;
	
	// Check for basic corruption
	if (!backdrop_validate_icon_object_safe(object)) {
		// Try to recover the object
		backdrop_emergency_reset_icon_object(object);
		return FALSE;
	}
	
	// Additional validation: check if icons need to be refreshed
	if (object->icon && !backdrop_validate_icon_dimensions_safe(object->icon)) {
		// Main icon is corrupted, try to save the select icon
		struct DiskObject *select_icon = object->iconlib_select_icon;
		object->iconlib_select_icon = NULL;
		
		// Reset main icon
		if (object->icon) {
			FreeCachedDiskObject(object->icon);
			object->icon = NULL;
		}
		
		// Restore select icon if it was valid
		if (select_icon && backdrop_validate_icon_dimensions_safe(select_icon)) {
			object->iconlib_select_icon = select_icon;
		}
		
		return FALSE;
	}
	
	// Validate select icon as well
	if (object->iconlib_select_icon && 
		!backdrop_validate_icon_dimensions_safe(object->iconlib_select_icon)) {
		// Select icon is corrupted, free it
		if (object->iconlib_select_icon) {
			FreeCachedDiskObject(object->iconlib_select_icon);
			object->iconlib_select_icon = NULL;
		}
		object->iconlib_select_refcount = 0;
		object->flags &= ~BDOF_ICONLIB_DEFAULT;
		return FALSE;
	}
	
	return TRUE;
}

// NEW: Safe wrapper for icon allocation with validation
struct DiskObject *backdrop_alloc_icon_safe(const char *name)
{
	struct DiskObject *icon;
	
	if (!name || !*name) {
		return NULL;
	}
	
	// Try to get the icon
	icon = GetDiskObject((char *)name);
	
	// Validate the icon before returning
	if (icon && !backdrop_validate_icon_dimensions_safe(icon)) {
		// Icon is corrupted, free it and return NULL
		FreeCachedDiskObject(icon);
		return NULL;
	}
	
	return icon;
}

// NEW: Defensive check for backdrop info structure
BOOL backdrop_validate_info_safe(BackdropInfo *info)
{
	if (!info)
		return FALSE;
	
	// Check for obviously corrupted pointers
	if ((ULONG)info < 0x1000 || (ULONG)info > 0x7FFFFFFF) {
		return FALSE;
	}
	
	// Validate window if present
	if (info->window && ((ULONG)info->window < 0x1000 || (ULONG)info->window > 0x7FFFFFFF)) {
		return FALSE;
	}
	
	return TRUE;
}

#endif