module AnotherModule (
    input  logic A[4],
    output logic B
);

    assign B = A[0] & A[1] | A[2] & A[3];

endmodule
