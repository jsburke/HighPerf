function graph_2d(file)
% Should alter to rename axes
data = csvread(file);
figure;
hold on;
plot1 = plot(data(:,1), data(:,2));
Leg1 = 'Forward';
plot2 = plot(data(:,1), data(:,3));
Leg2 = 'Reverse';
legend([plot1; plot2], [Leg1; Leg2]);
end
