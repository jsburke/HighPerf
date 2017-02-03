function graph_mmm(file)
data = csvread(file);
figure;
hold on;
plot1 = plot(data(:,1), (data(:,2)),'b');
plot2 = plot(data(:,1), (data(:,3)),'r');
plot3 = plot(data(:,1), (data(:,4)),'g');
legend('ijk','kij','jki','Location','southeast')
end
