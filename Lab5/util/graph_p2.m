function graph_p2(x, norm, rev, bs)

figure;
hold on;
plot1 = plot(x, norm, 'b');
leg1  = 'Normal';
plot2 = plot(x, rev, 'r');
leg2  = 'Interchange';
plot3 = plot(x, bs, 'g');
leg3  = 'Blocked';
legend(leg1, leg2, leg3);
end