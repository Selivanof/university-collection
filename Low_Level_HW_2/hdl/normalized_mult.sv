module normalization_module (
    input  logic [47:0] P,                      // 48-bit long multiplication result
    input  logic [9:0]  sum_exp,                // 10-bit long sum of exponents subtracted by the bias
    output logic        guard_bit, sticky_bit,  // 1-bit  guard and sticky bits
    output logic [22:0] norm_mantissa,          // 23-bit long normalized mantissa
    output logic [9:0]  norm_exp                // 10-bit normalized exponent
);

always_comb begin
    norm_exp = sum_exp + P[47];
    if (P[47]) begin
        norm_mantissa = P[46:24];
        guard_bit     = P[23];
        sticky_bit    = |P[22:0];
    end 
    else begin
        norm_mantissa = P[45:23];
        guard_bit     = P[22];
        sticky_bit    = |P[21:0];
    end   
end

endmodule