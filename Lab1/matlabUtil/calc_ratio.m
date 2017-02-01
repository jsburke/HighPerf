function out = calc_ratio(file)
a = csvread(file);
out = a(:,2)./a(:,3);
end
