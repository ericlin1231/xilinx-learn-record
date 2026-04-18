`timescale 1ns / 1ps
module hdmi_data_gen
(
    input            pix_clk,
    output [7:0]     VGA_R,
    output [7:0]     VGA_G,
    output [7:0]     VGA_B,
    output           VGA_HS,
    output           VGA_VS,
    output           VGA_DE
);

parameter H_Active = 1920;
parameter H_Front  = 88;
parameter H_Sync   = 44;
parameter H_Back   = 148;
parameter H_Total  = H_Active + H_Front + H_Sync + H_Back;

parameter V_Active = 1080;
parameter V_Front  = 4;
parameter V_Sync   = 5;
parameter V_Back   = 36;
parameter V_Total  = V_Active + V_Front + V_Sync + V_Back;

parameter H_Sync_Start = H_Active + H_Front;
parameter H_Sync_End   = H_Active + H_Front + H_Sync;

parameter V_Sync_Start = V_Active + V_Front;
parameter V_Sync_End   = V_Active + V_Front + V_Sync;

reg [11:0] x_cnt = 12'd0;
always @(posedge pix_clk) begin
    if (x_cnt == H_Total - 1) x_cnt <= 0;
    else                      x_cnt <= x_cnt + 1;
end

reg [11:0] y_cnt = 12'd0;
always @(posedge pix_clk) begin
    if (x_cnt == H_Total - 1) begin
        if (y_cnt == V_Total - 1) y_cnt <= 0;
        else                      y_cnt <= y_cnt + 1;
    end
end

wire hs_de = (x_cnt < H_Active);
wire vs_de = (y_cnt < V_Active);
wire de    = hs_de && vs_de;

assign VGA_HS = (x_cnt >= H_Sync_Start && x_cnt < H_Sync_End) ? 1'b0 : 1'b1;
assign VGA_VS = (y_cnt >= V_Sync_Start && y_cnt < V_Sync_End) ? 1'b0 : 1'b1;

parameter BLOCK_WIDTH  = 200;
parameter BLOCK_HEIGHT = 200;
parameter BLOCK_SPEED  = 8;

reg [11:0] block_x = 12'd100;
reg [11:0] block_y = 12'd100;
reg        dir_x   = 1'b0;
reg        dir_y   = 1'b0;

wire frame_update = (x_cnt == H_Total - 1) && (y_cnt == V_Total - 1);

always @(posedge pix_clk) begin
    if (frame_update) begin
        if (dir_x == 1'b0) begin
            if (block_x + BLOCK_WIDTH + BLOCK_SPEED >= H_Active) dir_x <= 1'b1;
            else block_x <= block_x + BLOCK_SPEED;
        end else begin
            if (block_x <= BLOCK_SPEED) dir_x <= 1'b0;
            else block_x <= block_x - BLOCK_SPEED;
        end

        if (dir_y == 1'b0) begin
            if (block_y + BLOCK_HEIGHT + BLOCK_SPEED >= V_Active) dir_y <= 1'b1;
            else block_y <= block_y + BLOCK_SPEED;
        end else begin
            if (block_y <= BLOCK_SPEED) dir_y <= 1'b0;
            else block_y <= block_y - BLOCK_SPEED;
        end
    end
end

wire in_block = de
             && (x_cnt >= block_x) && (x_cnt < block_x + BLOCK_WIDTH)
             && (y_cnt >= block_y) && (y_cnt < block_y + BLOCK_HEIGHT);

reg [8:0] color_cnt = 9'd0;

always @(posedge pix_clk) begin
    if (frame_update) begin
        color_cnt <= color_cnt + 1;
    end
end

function [7:0] triangle;
    input [8:0] val;
    begin
        triangle = val[8] ? ~val[7:0] : val[7:0];
    end
endfunction

wire [7:0] r_val = triangle(color_cnt);
wire [7:0] g_val = triangle(color_cnt + 9'd170);
wire [7:0] b_val = triangle(color_cnt + 9'd340);
reg [7:0] VGA_R_reg, VGA_G_reg, VGA_B_reg;

always @(posedge pix_clk) begin
    if (in_block) begin
        VGA_R_reg <= r_val;
        VGA_G_reg <= g_val;
        VGA_B_reg <= b_val;
    end else begin
        VGA_R_reg <= 8'h00;
        VGA_G_reg <= 8'h00;
        VGA_B_reg <= 8'h00;
    end
end

assign VGA_DE = de;
assign VGA_R  = de ? VGA_R_reg : 8'h00;
assign VGA_G  = de ? VGA_G_reg : 8'h00;
assign VGA_B  = de ? VGA_B_reg : 8'h00;

endmodule