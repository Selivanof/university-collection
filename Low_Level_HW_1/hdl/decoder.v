module decoder(
  input wire btnc,             // Button Center
  input wire btnl,            // Button Left
  input wire btnr,            // Button Right
  output wire [3:0] alu_op    // Decoded alu_op
);

// Not versions of Inputs
wire not_btnc, not_btnl, not_btnr;
assign not_btnc = ~btnc;
assign not_btnl = ~btnl;
assign not_btnr = ~btnr;

// Decoding of alu_op[0]
wire bit0_and1, bit0_and2, bit0_xor1;

assign bit0_and1 = btnl & not_btnr;
assign bit0_xor1 = btnl ^ btnc;
assign bit0_and2 = btnr & bit0_xor1;

assign alu_op[0] = bit0_and1 | bit0_and2;

// Decoding of alu_op[1]
wire bit1_and1, bit1_and2;

assign bit1_and1 = btnr & btnl;
assign bit1_and2 = not_btnl & not_btnc;

assign alu_op[1] = bit1_and1 | bit1_and2;


// Decoding of alu_op[2]
wire bit2_and2, bit2_xor1, bit2_or1;

assign bit2_and2 = btnr & btnl;
assign bit2_xor1 = btnr ^ btnl;
assign bit2_or1 = bit2_and2 | bit2_xor1;

assign alu_op[2] = not_btnc & bit2_or1;


// Decoding of alu_op[3]
wire bit3_xnor1, bit3_and1, bit3_or1;

assign bit3_xnor1 = btnr ~^ btnc;
assign bit3_and1 = btnc & not_btnr;
assign bit3_or1 = bit3_xnor1 | bit3_and1;

assign alu_op[3] = btnl & bit3_or1;

endmodule