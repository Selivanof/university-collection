package float_interpretations;
    // Different types of 32-bit float numbers
    typedef enum logic [2:0] {
        ZERO,
        INF,
        NORM,
        MIN_NORM,
        MAX_NORM
    } interp_t; 
endpackage

package rounding_modes;
    // All available rounding modes
    typedef enum {
        IEEE_near,
        IEEE_zero,
        IEEE_pinf,
        IEEE_ninf,
        near_up,
        away_zero
    } rounding_mode_t;
endpackage