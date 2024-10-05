import rounding_modes::*;

module fp_mult (
       input  logic           [31:0]    a, b,   // 32-bit operands
       input  rounding_mode_t           rnd,    // 3-bit rounding mode input
       output logic           [31:0]    z,      // 32-bit multiplication result
       output logic           [7:0]     status  // 8-bit status output
);

// Helper Signals
logic a_sign;
logic b_sign;
logic z_sign;
logic [7:0] a_exponent;
logic [7:0] b_exponent;
logic [22:0] a_mantissa;
logic [22:0] b_mantissa;
logic [23:0] a_extended_mantissa;
logic [23:0] b_extended_mantissa;
// Stage 1 Helper Signals
logic [9:0] exponent_sum;
logic [47:0] mantissa_prod;
// Overflow and Underflow bits
logic overflow_bit, underflow_bit;

// ===== NORMALIZATION MODULE  =====
// Helper Signals
logic [22:0]        normalized_mantissa;
logic signed [9:0]  normalized_exponent;   
logic               sticky_bit;
logic               guard_bit;
// Module Instance
normalization_module norm_inst (
    mantissa_prod,
    exponent_sum,
    guard_bit,
    sticky_bit,
    normalized_mantissa,
    normalized_exponent
);

// ===== ROUNDING MODULE  =====
// Helper Signals
logic [24:0]        rounding_result;
logic               rounding_inexact;
logic [23:0]        post_round_mantissa;
logic signed [9:0]  post_round_exponent;
// Module Instance
rounding_module rounding_inst (
    {1'b1, normalized_mantissa},
    rnd,
    guard_bit,
    sticky_bit,
    z_sign,
    rounding_result,
    rounding_inexact
);

// ===== EXCEPTION MODULE  =====
// Helper Signals
logic [31:0] z_calc;
logic zero_f, inf_f, nan_f, tiny_f, huge_f, inexact_f;
// Module Instance
exception_mult exception_inst(
    a,
    b,
    z_calc,
    overflow_bit,
    underflow_bit,
    rounding_inexact,
    rnd,
    z,
    zero_f,
    inf_f,
    nan_f,
    tiny_f,
    huge_f,
    inexact_f
);

// Float Decoding
// Extract the sign field
assign a_sign = a[31];
assign b_sign = b[31];
// Extract the 8-bit exponent field
assign a_exponent = a[30:23];
assign b_exponent = b[30:23];
// Extract the 23-bit mantissa (fraction) field
assign a_mantissa = a[22:0];
assign b_mantissa = b[22:0];
// Add the leading one to the mantissa
assign a_extended_mantissa = {1'b1, a_mantissa};
assign b_extended_mantissa = {1'b1, b_mantissa};

// Status composition
assign status = {0, 0, inexact_f, huge_f, tiny_f, nan_f, inf_f, zero_f};

// Basic Operations before normalization/rounding
assign z_sign = a_sign ^ b_sign;
assign exponent_sum = (a_exponent + b_exponent) - 127;
assign mantissa_prod = a_extended_mantissa * b_extended_mantissa;

// Overflow/Underflow Detenction
assign underflow_bit = normalized_exponent < 1;
assign overflow_bit = post_round_exponent > 254;

// Post Rounding Signals
always_comb begin
    if (rounding_result[24] == 1'b1)
    begin
        post_round_mantissa = rounding_result[24:1];
        post_round_exponent = normalized_exponent + 1;
    end else begin
        post_round_mantissa = rounding_result [23:0];
        post_round_exponent = normalized_exponent;
    end
    z_calc = {z_sign, post_round_exponent[7:0], post_round_mantissa[22:0]};
end


endmodule