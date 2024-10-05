// ===================== STATUS BITS TESTING ===================== 
module test_status_bits(
    input logic [7:0] status,
    input logic rst
    );

    // Status signal decoding
    logic zero_f, inf_f, nan_f, tiny_f, huge_f, inexact_f;

    assign zero_f    = status[0];
    assign inf_f     = status[1];
    assign nan_f     = status[2];
    assign tiny_f    = status[3];
    assign huge_f    = status[4];
    assign inexact_f = status[5];
  
    always_comb begin
        if(!(rst || $isunknown(status)))  // Don't assert if reset is active or the status is unknown
	    begin
            // Zero and Inf cannot be true at the same time
            assert  (!(zero_f && inf_f))  else $error("Error: 'zero' and 'inf' both asserted.");
            // When NaN occurs, result is inf. It cannot be zero or inexact. Overflow and Underflow should also not occur.
            assert  (!(nan_f && zero_f))  else $error("Error: 'NaN' and 'zero' both asserted.");
            assert  (!(nan_f && inexact_f)) else $error("Error: 'NaN' and 'inexact' both asserted.");
            assert  (!(nan_f && huge_f))    else $error("Error: 'NaN' and 'huge' both asserted.");
            assert  (!(nan_f && tiny_f))    else $error("Error: 'NaN' and 'tiny' both asserted.");
            // Underflow will result to either zero or minNorm - Cannot be inf or NaN (NaN only results from 0 x inf)
            assert  (!(tiny_f && inf_f))  else $error("Error: 'tiny' and 'inf' both asserted."); 
            // Overflow will result to either inf or maxNorm - Cannot be zero or NaN (NaN only results from 0 x inf)
            assert  (!(huge_f && zero_f)) else $error("Error: 'huge' and 'zero' both asserted.");
            // Overflow and Underflow can not occur at the same time
            assert  (!(huge_f && tiny_f)) else $error("Error: 'huge' and 'tiny' both asserted.");
            // Leftover bits should always be 0
            assert  (!(status[6])) else $error("Error: Status bit 6 reserved to 0");
            assert  (!(status[7])) else $error("Error: Status bit 7 reserved to 0 (No division)");
	    end
    end

endmodule

// ============= STATUS - Z/A/B COMBINATIONS TESTING ============= 
module test_status_z_combinations(
  input logic         clk, rst,       // 1-bit Clock and Reset signals
  input logic [7:0]   status,         // 8-bit Status
  input logic [31:0]  a, b, z         // 32-bit Operands (a,b) and Result (z)
);
  
  // Internal Helper Signals
  logic zero_f, inf_f, nan_f, tiny_f, huge_f, inexact_f;
  logic [7:0] z_exponent, a_exponent, b_exponent;
  logic [22:0] z_mantissa;

  // Decoding Status
  assign zero_f    = status[0];
  assign inf_f     = status[1];
  assign nan_f     = status[2];
  assign tiny_f    = status[3];
  assign huge_f    = status[4];
  assign inexact_f = status[5];

  // Getting exponents
  assign z_exponent = z[30:23];
  assign a_exponent = a[30:23];
  assign b_exponent = b[30:23];

  // Getting result mantissa
  assign z_mantissa = z[22:0];

  // Disable assertions while reset is high or the status signal is unknown
  default disable iff(rst || $isunknown(status));

  // SEQUENCE DEFINITIONS
  sequence zeroResult;
    z_exponent == 8'b0;
  endsequence
  sequence infResult;
    z_exponent == 8'b11111111;
  endsequence
  sequence maxNormResult;
    z_exponent[7:1] == 7'b1111111 && z_exponent[0] == 0 && z_mantissa == {23{1'b1}};
  endsequence
  sequence minNormResult;
    z_exponent[7:1] == 7'b0 && z_exponent[0] == 1 && z_mantissa == 23'b0;
  endsequence
  sequence aWasZero;
    $past(a_exponent, 2) == 8'b0;
  endsequence  
  sequence aWasInf;
    $past(a_exponent, 2) == 8'b11111111;
  endsequence  
  sequence bWasZero;
    $past(b_exponent, 2) == 8'b0;
  endsequence  
  sequence bWasInf;
    $past(b_exponent, 2) == 8'b11111111;
  endsequence  

  // PROPERTIES DEFINITION
  property zero_bit;
    @(posedge clk) zero_f |-> zeroResult;
  endproperty
  property inf_bit;
    @(posedge clk) inf_f |-> infResult;
  endproperty
  property nan_bit;
    @(posedge clk) nan_f |-> ((aWasZero and bWasInf) or (aWasInf and bWasZero));
  endproperty
  property huge_bit;
    @(posedge clk) huge_f |-> (infResult or maxNormResult);
  endproperty
  property tiny_bit;
    @(posedge clk) tiny_f |-> (zeroResult or minNormResult);
  endproperty

  // PROPERTIES ASSERTMENT
  assert property (zero_bit) else $error("Error: Zero bit set with incorrect exponent");
  assert property (inf_bit) else $error("Error: Inf bit set with incorrect exponent");
  assert property (nan_bit) else $error("Error: NaN bit set but the previous operation was not 0xINF.");
  assert property (huge_bit) else $error("Error: Huge bit set but z is neither inf nor maxNorm.");
  assert property (tiny_bit) else $error("Error: Tiny bit set but z is neither zero nor minNorm.");

endmodule