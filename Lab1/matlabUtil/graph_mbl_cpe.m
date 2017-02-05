function graph_mmm(file)
data = csvread(file);
figure;
% hold on; % makes a plot with linear (not log-log) axes
plot1 = plot(data(:,1), (data(:,2)),'m');
hold on; % works
plot2 = plot(data(:,1), (data(:,3)),'b');
plot3 = plot(data(:,1), (data(:,4)),'c');
plot4 = plot(data(:,1), (data(:,5)),'g');
plot5 = plot(data(:,1), (data(:,6)),'y');
plot6 = plot(data(:,1), (data(:,7)),'r');
% hold on; % only plots one curve and generates 'Ifgnoring extra legend entries' errors
legend('2','4','8','16','32','64')
end
