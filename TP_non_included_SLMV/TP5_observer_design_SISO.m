%% TP5 - Observer Design for a SISO System
% Module: Linear Multivariable Systems
% System: Mass-Spring-Damper
% Objective:
% Design a Luenberger observer to estimate the full state vector
% from the measured output.

clear;
clc;
close all;

%% 1. Save path

mainFolder = 'D:\S3 MTA\7- SLMV & SED & Commande Machine\SLMV\TPs';
outputFolder = fullfile(mainFolder, 'TP5_SISO_results');

if ~exist(outputFolder, 'dir')
    mkdir(outputFolder);
end

cd(mainFolder);

%% 2. Physical parameters

m = 1;      % Mass in kg
c = 2;      % Damping coefficient in N.s/m
k = 10;     % Spring stiffness in N/m

%% 3. State-space matrices

% State vector:
% x1 = position x
% x2 = velocity dx/dt
%
% Input:
% u = F
%
% Output:
% y = x1

A = [0      1;
    -k/m   -c/m];

B = [0;
     1/m];

C = [1 0];

D = 0;

%% 4. Create state-space system

sys = ss(A, B, C, D);

%% 5. Check observability before observer design

Ob = obsv(A, C);
rank_Ob = rank(Ob);
n = size(A,1);

fprintf('Rank of observability matrix = %d\n', rank_Ob);

if rank_Ob == n
    disp('The system is observable. Observer design is possible.');
else
    error('The system is not observable. Observer design is not possible.');
end

%% 6. Open-loop poles

open_loop_poles = eig(A);

disp('Open-loop poles:');
disp(open_loop_poles);

%% 7. Desired observer poles

% Observer poles are chosen faster than the system poles.
% Open-loop poles are approximately -1 +/- 3j.
% We choose real stable observer poles.

observer_poles = [-8 -9];

disp('Desired observer poles:');
disp(observer_poles);

%% 8. Observer gain L

% For observer design:
% L = place(A', C', observer_poles)'

L = place(A', C', observer_poles)';

disp('Observer gain L:');
disp(L);

%% 9. Observer error dynamics

A_observer_error = A - L*C;

observer_error_poles = eig(A_observer_error);

disp('Observer error dynamics matrix A - L*C:');
disp(A_observer_error);

disp('Observer error poles:');
disp(observer_error_poles);

%% 10. Simulation of real states and estimated states

% Augmented system:
%
% x_dot     = A*x + B*u
% xhat_dot  = A*xhat + B*u + L*(y - C*xhat)
%
% Since y = C*x:
%
% xhat_dot = L*C*x + (A - L*C)*xhat + B*u

A_aug = [A           zeros(2,2);
         L*C         A - L*C];

B_aug = [B;
         B];

C_aug = eye(4);

D_aug = zeros(4,1);

sys_aug = ss(A_aug, B_aug, C_aug, D_aug);

%% 11. Simulation parameters

t = 0:0.01:5;

% Input force
u = ones(size(t));

% Initial condition:
% Real system starts from position 1 m and velocity 0 m/s.
% Observer starts from zero initial estimation.

x0_real = [1;
           0];

x0_estimated = [0;
                0];

x0_aug = [x0_real;
          x0_estimated];

%% 12. Run simulation

[z, t_sim] = lsim(sys_aug, u, t, x0_aug);

x1_real = z(:,1);
x2_real = z(:,2);

x1_estimated = z(:,3);
x2_estimated = z(:,4);

error_x1 = x1_real - x1_estimated;
error_x2 = x2_real - x2_estimated;

%% 13. Plot real and estimated position

figure;
plot(t_sim, x1_real, 'LineWidth', 1.5);
hold on;
plot(t_sim, x1_estimated, '--', 'LineWidth', 1.5);
grid on;
legend('Real position x1', 'Estimated position xhat1');
title('TP5 - Real and Estimated Position');
xlabel('Time (s)');
ylabel('Position');

%% 14. Plot real and estimated velocity

figure;
plot(t_sim, x2_real, 'LineWidth', 1.5);
hold on;
plot(t_sim, x2_estimated, '--', 'LineWidth', 1.5);
grid on;
legend('Real velocity x2', 'Estimated velocity xhat2');
title('TP5 - Real and Estimated Velocity');
xlabel('Time (s)');
ylabel('Velocity');

%% 15. Plot estimation errors

figure;
plot(t_sim, error_x1, 'LineWidth', 1.5);
hold on;
plot(t_sim, error_x2, 'LineWidth', 1.5);
grid on;
legend('Error e1 = x1 - xhat1', 'Error e2 = x2 - xhat2');
title('TP5 - Estimation Errors');
xlabel('Time (s)');
ylabel('Estimation error');

%% 16. Pole-zero map of observer error dynamics

sys_observer_error = ss(A_observer_error, eye(2), eye(2), zeros(2,2));

figure;
pzmap(sys_observer_error);
grid on;
title('TP5 - Observer Error Dynamics Poles');

%% 17. Save all figures

figHandles = findall(0, 'Type', 'figure');

for i = 1:length(figHandles)
    fig = figHandles(i);
    figure(fig);

    filename_png = fullfile(outputFolder, sprintf('TP5_SISO_Figure_%d.png', i));
    saveas(fig, filename_png);

    filename_fig = fullfile(outputFolder, sprintf('TP5_SISO_Figure_%d.fig', i));
    savefig(fig, filename_fig);
end

disp('All TP5 figures have been saved.');

%% 18. Save numerical results in a text file

resultsFile = fullfile(outputFolder, 'TP5_SISO_results.txt');

diary(resultsFile);

disp('==============================================');
disp('TP5 - Observer Design for a SISO System');
disp('==============================================');

disp('Physical parameters:');
fprintf('m = %.2f kg\n', m);
fprintf('c = %.2f N.s/m\n', c);
fprintf('k = %.2f N/m\n', k);

disp('Matrix A:');
disp(A);

disp('Matrix B:');
disp(B);

disp('Matrix C:');
disp(C);

disp('Matrix D:');
disp(D);

disp('Observability matrix:');
disp(Ob);

fprintf('Rank of observability matrix = %d\n', rank_Ob);

disp('Open-loop poles:');
disp(open_loop_poles);

disp('Desired observer poles:');
disp(observer_poles);

disp('Observer gain L:');
disp(L);

disp('Observer error dynamics matrix A - L*C:');
disp(A_observer_error);

disp('Observer error poles:');
disp(observer_error_poles);

disp('Initial real state:');
disp(x0_real);

disp('Initial estimated state:');
disp(x0_estimated);

diary off;

%% 19. Save MATLAB workspace

save(fullfile(outputFolder, 'TP5_SISO_workspace.mat'));

disp('TP5 SISO results have been saved successfully.');
disp(['Results folder: ', outputFolder]);