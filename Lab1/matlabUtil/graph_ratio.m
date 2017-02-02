function graph_ratio(file)
data = csvread(file);
figure;
hold on;
plot1 = plot(data(:,1), (data(:,2)./data(:,3)));
Leg1 = 'Speed up';
legend([plot1], [Leg1]);
end