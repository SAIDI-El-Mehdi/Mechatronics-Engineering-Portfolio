clear all;
clc;
close all;

% Question 1
f = @(x, y) y - 2*x / y;         
y_exact = @(x) sqrt(2*x + 1);    
x0 = 0; xf = 1;                  
y0 = 1;                         
N = 100;                        
h = (xf - x0) / N;               
x = x0:h:xf;                     

y_rk = zeros(1, N+1);           
y_rk(1) = y0;                   

% Boucle pour appliquer RK4
for n = 1:N
    k1 = f(x(n), y_rk(n));
    k2 = f(x(n) + h/2, y_rk(n) + h*k1/2);
    k3 = f(x(n) + h/2, y_rk(n) + h*k2/2);
    k4 = f(x(n) + h, y_rk(n) + h*k3);
    y_rk(n+1) = y_rk(n) + h/6 * (k1 + 2*k2 + 2*k3 + k4);
end


y_exact_values = y_exact(x);

% Question 2: Tracé des résultats
figure;
plot(x, y_rk, '-o', 'DisplayName', 'Runge-Kutta ');
hold on;
plot(x, y_exact_values, 'b', 'DisplayName', 'Solution exacte');
xlabel('x');
ylabel('y');
legend('show');  
title('Résolution de y'' = y - 2x / y');
grid on;
