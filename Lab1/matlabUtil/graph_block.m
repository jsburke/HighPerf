function graph_block(file1,file2)
data = csvread(file1);
data2 = csvread(file2);
figure;
hold on;
plot1 = plot(data(:,1), (data(:,2)),'b');
Leg1 = 'Non-blocking';
plot2 = plot(data2(:,1), (data2(:,2)),'r');
Leg2 = 'Blocking    ';
legend([plot1; plot2], [Leg1; Leg2]);
end
