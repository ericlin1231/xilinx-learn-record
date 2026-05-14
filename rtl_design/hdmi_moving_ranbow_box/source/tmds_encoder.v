
`timescale 1ns / 1ps
module tmds_encoder (
    input            clk,
    input      [7:0] data,
    input      [1:0] c,
    input            de,
    output reg [9:0] q_out
);

    wire [3:0] n1d = data[0] + data[1] + data[2] + data[3]
                   + data[4] + data[5] + data[6] + data[7];
    wire use_xnor = (n1d > 4'd4) || (n1d == 4'd4 && data[0] == 1'b0);

    reg [8:0] q_m;
    integer i;
    always @* begin
        q_m[0] = data[0];
        for (i = 1; i < 8; i = i + 1)
            q_m[i] = use_xnor ? (q_m[i-1] ~^ data[i]) : (q_m[i-1] ^ data[i]);
        q_m[8] = ~use_xnor;
    end

    wire [3:0] n1qm = q_m[0] + q_m[1] + q_m[2] + q_m[3]
                    + q_m[4] + q_m[5] + q_m[6] + q_m[7];
    wire [3:0] n0qm = 4'd8 - n1qm;

    reg signed [4:0] cnt;

    always @(posedge clk) begin
        if (!de) begin
            cnt <= 5'sd0;
            case (c)
                2'b00: q_out <= 10'b1101010100;
                2'b01: q_out <= 10'b0010101011;
                2'b10: q_out <= 10'b0101010100;
                2'b11: q_out <= 10'b1010101011;
            endcase
        end
        else begin
            if (cnt == 0 || n1qm == n0qm) begin
                q_out[9]   <= ~q_m[8];
                q_out[8]   <=  q_m[8];
                q_out[7:0] <=  q_m[8] ? q_m[7:0] : ~q_m[7:0];
                cnt <= q_m[8] ? cnt + $signed(n1qm) - $signed(n0qm)
                              : cnt + $signed(n0qm) - $signed(n1qm);
            end
            else if ((cnt > 0 && n1qm > n0qm) ||
                     (cnt < 0 && n0qm > n1qm)) begin
                q_out[9]   <= 1'b1;
                q_out[8]   <= q_m[8];
                q_out[7:0] <= ~q_m[7:0];
                cnt <= cnt + {3'b0, q_m[8], 1'b0}
                           + $signed(n0qm) - $signed(n1qm);
            end
            else begin
                q_out[9]   <= 1'b0;
                q_out[8]   <= q_m[8];
                q_out[7:0] <= q_m[7:0];
                cnt <= cnt - {3'b0, ~q_m[8], 1'b0}
                           + $signed(n1qm) - $signed(n0qm);
            end
        end
    end

endmodule
