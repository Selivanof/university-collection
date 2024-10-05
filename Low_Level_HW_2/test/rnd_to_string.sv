import rounding_modes::*;

// Converts the rounding mode enum to the respective string (for usage with multiplication.sv)
function string rnd_to_string(rounding_mode_t mode);
    case (mode)
        IEEE_near:  return "IEEE_near";
        IEEE_zero:  return "IEEE_zero";
        IEEE_pinf:  return "IEEE_pinf";
        IEEE_ninf:  return "IEEE_ninf";
        near_up:    return "near_up";
        away_zero:  return "away_zero";
        default:    return "IEEE_near";
    endcase
endfunction