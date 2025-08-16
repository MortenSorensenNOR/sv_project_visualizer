module SubModule (
    input  logic A,
    output logic B
);
    assign B = ~A;
endmodule

module TestModule (
    input logic clk,
    input logic rstn,

    input  logic A,
    output logic B
);
    SubModule module_inst (
        .A(A),
        .B(B)
    );
endmodule
