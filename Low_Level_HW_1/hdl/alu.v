module alu
(
	input [31:0] op1,
	input [31:0] op2,
	input [3:0] alu_op,
	output reg zero,
	output reg [31:0] result
);


// =========================== Operations ===========================

parameter[3:0] ALUOP_AND = 4'b0000;  	//AND
parameter[3:0] ALUOP_OR  = 4'b0001;		//OR
parameter[3:0] ALUOP_XOR = 4'b1101;		//XOR
parameter[3:0] ALUOP_ADD = 4'b0010;		//Addition
parameter[3:0] ALUOP_SUB = 4'b0110;		//Subtraction
parameter[3:0] ALUOP_LT  = 4'b0111;		//Less than
parameter[3:0] ALUOP_LRS = 4'b1000;		//Logical Right Shift
parameter[3:0] ALUOP_LLS = 4'b1001;		//Logical Left Shift
parameter[3:0] ALUOP_ARS = 4'b1010;		//Arithmetic Right Shift


// ============================== ALU ==============================

reg signed [31:0] sig_op1,sig_op2;

always @(*)
begin
	// Signed Operators
	sig_op1 = op1;
    sig_op2 = op2;
	// Operations
	case (alu_op)
		ALUOP_AND: result = op1 & op2;
		ALUOP_OR: result = op1 | op2;
		ALUOP_XOR: result = op1 ^ op2;
		ALUOP_ADD: result = sig_op1 + sig_op2;
		ALUOP_SUB: result = sig_op1 - sig_op2;
		ALUOP_LT: result = (sig_op1 < sig_op2) ? 1 : 0;
		ALUOP_LRS: result = op1 >> op2[4:0];
		ALUOP_LLS: result = op1 << op2[4:0];
		ALUOP_ARS: result = sig_op1 >>> op2[4:0];
		default: result = 32'hxxxxxxxx;
	endcase
	// Zero Assignment
	zero = (result == 0);
end

endmodule