clear all;
clc;
close all;

% Question 1
x = [0, 0.3, 0.6, 0.8, 1];
y = [0, 0.2281, 0.3609, 0.4177, 0.4570];
n = length(x) - 1;
Pn = polyfit(x, y, n);
disp(Pn);
% Question 2
xvar= 0: 0.04: 0.08;
syms z;
L = 0;
for i = 1:length(x)
    Li = 1;
    for j = 1:length(x)
        if j ~= i
            Li = Li * (z - x(j)) / (x(i) - x(j));
        end
    end
    L = L + Li * y(i);
end
disp(L);
% Question 3
xvar = 0:0.04:0.8; 
y_L = double(subs(L, z, xvar)); 
y_Pn = polyval(Pn, xvar);
figure;
hold on;
grid on;

% Tracer PL
plot(xvar, y_L, 'r');


plot(xvar, y_Pn, 'b');


plot(x, y, 'ko');


title('Interpolation par Lagrange et Newton');
xlabel('x');
ylabel('y');



