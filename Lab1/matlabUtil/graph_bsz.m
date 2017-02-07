function graph_bsz(f1,f2,f3,f4,f5,f6,f7,f8,f9)
d1 = csvread(f1);
d2 = csvread(f2);
d3 = csvread(f3);
d4 = csvread(f4);
d5 = csvread(f5);
d6 = csvread(f6);
d7 = csvread(f7);
d8 = csvread(f8);
d9 = csvread(f9);

figure;
hold on;
p1 = plot(d1(:,1), (d1(:,2)),'b');
Leg1 = 'Block - 1 ';
p2 = plot(d2(:,1), (d2(:,2)),'r-o');
Leg2 = 'Block - 2 ';
p3 = plot(d3(:,1), (d3(:,2)),'g-+');
Leg3 = 'Block - 4 ';
p4 = plot(d4(:,1), (d4(:,2)),'k-*');
Leg4 = 'Block - 8 ';
p5 = plot(d5(:,1), (d5(:,2)),'m--');
Leg5 = 'Block - 12';
p6 = plot(d6(:,1), (d6(:,2)),'b-o');
Leg6 = 'Block - 16';
p7 = plot(d7(:,1), (d7(:,2)),'r-+');
Leg7 = 'Block - 20';
p8 = plot(d8(:,1), (d8(:,2)),'g--');
Leg8 = 'Block - 25';
p9 = plot(d9(:,1), (d9(:,2)),'k--');
Leg9 = 'Block - 32';

legend([p1;p2;p3;p4;p5;p6;p7;p8;p9], [Leg1;Leg2;Leg3;Leg4;Leg5;Leg6;Leg7;Leg8;Leg9]);
end
