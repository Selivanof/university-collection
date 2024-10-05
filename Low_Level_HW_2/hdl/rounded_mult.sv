import rounding_modes::*;

module rounding_module (
    input  logic [23:0]      mantissa,              // 24-bit post normalization mantissa
    input  rounding_mode_t   round,                 // 3-bit rounding mode
    input  logic             guard, sticky, sign,   // 1-bit input signals
    output logic [24:0]      result,                // 25-bit output rounded mantissa
    output logic             inexact                // 1-bit output signal
);

// Internal Signals
logic increase_mantissa;

// Calculation of inexact bit
assign inexact = guard | sticky;

// Rounding Logic
always_comb begin
    case (round)
        // IEEE_near case can be commented out since it is the default one - left for clarity
        IEEE_near: increase_mantissa = (guard && (sticky || (!sticky && mantissa[0])));
        IEEE_zero: increase_mantissa = 0;
        IEEE_pinf: increase_mantissa = (inexact & !sign);
        IEEE_ninf: increase_mantissa = (inexact & sign);
        near_up  : increase_mantissa = (guard && (sticky || (!sticky && !sign)));
        away_zero: increase_mantissa = inexact;
        default  : increase_mantissa = (guard && (sticky || (!sticky && mantissa[0])));
    endcase
    result = mantissa + increase_mantissa;
end


endmodule