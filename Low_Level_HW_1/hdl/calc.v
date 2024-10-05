module calc(
  input clk,                // Clock
  input btnc,               // Button Center
  input btnl,               // Button Left
  input btnu,               // Button Up
  input btnr,               // Button Right
  input btnd,               // Button Down
  input [15:0] sw,          // Input Switches
  output wire [15:0] led    // Output LED
);


// ================== Internal Signal Declarations ==================

// Accumulator
reg [15:0] accumulator;

//ALU
wire [31:0] calc_alu_results;
wire [31:0] calc_alu_op1;
wire [31:0] calc_alu_op2;
wire [3:0]  calc_alu_op;


// ======================= Operation Decoding =======================

decoder alu_op_decoder(
    .btnc(btnc),
    .btnl(btnl),
    .btnr(btnr),
    .alu_op(calc_alu_op)
);


// ============================== ALU ==============================

// Sign Extend Calculator Operands
assign calc_alu_op1 = {{16{accumulator[15]}}, accumulator};
assign calc_alu_op2 = {{16{sw[15]}}, sw};

alu calc_alu (
    .op1(calc_alu_op1),
    .op2(calc_alu_op2),
    .alu_op(calc_alu_op),
    .zero(),
    .result(calc_alu_results)
);


// ========================= Accumulator =========================

assign led = accumulator[15:0];

always@(posedge clk)
begin
    // Reset Accumulator
    if (btnu) accumulator <= 16'b0;       
end

always@(posedge btnd)
begin
    // Update accumulator if not being reset
    if(~btnu) accumulator <= calc_alu_results[15:0]; 
end

endmodule