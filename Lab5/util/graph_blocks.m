function graph_blocks(x, p1, p2, p3, p4)

figure;
hold on;
plot1 = plot(x, p1, 'b');
leg1  = 'bs = 5';
plot2 = plot(x, p2, 'r');
leg2  = 'bs = 8';
plot3 = plot(x, p3, 'g');
leg3  = 'bs = 16';
plot4 = plot(x, p4, 'k');
leg4  = 'bs =25';
legend(leg1, leg2, leg3, leg4);
end