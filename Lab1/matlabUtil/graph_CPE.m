function graph_CPE(file)
data = csvread(file);
figure;
hold on;
plot1 = plot(data(:,1), (data(:,2)./data(:,1)),'r');
Leg1 = 'Forward CPE';
plot2 = plot(data(:,1), (data(:,3)./data(:,1)),'b');
Leg2 = 'Reverse CPE';
legend([plot1; plot2], [Leg1; Leg2]);
end