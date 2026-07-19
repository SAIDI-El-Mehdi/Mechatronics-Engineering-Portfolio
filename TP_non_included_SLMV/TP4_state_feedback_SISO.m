%% TP4 - State Feedback Control of a SISO System
% Module: Linear Multivariable Systems
% System: Mass-Spring-Damper
% Objective:
% Design a state feedback controller using pole placement and compare
% open-loop and closed-loop responses.

clear;
clc;
close all;

%% 1. Save path

mainFolder = 'D:\S3 MTA\7- SLMV & SED & Commande Machine\SLMV\TPs';
outputFolder = fullfile(mainFolder, 'TP4_SISO_results');

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

%% 4. Open-loop system

sys_open = ss(A, B, C, D);

disp('Open-loop poles:');
open_loop_poles = pole(sys_open);
disp(open_loop_poles);

%% 5. Check controllability before pole placement

Co = ctrb(A, B);
rank_Co = rank(Co);
n = size(A,1);

fprintf('Rank of controllability matrix = %d\n', rank_Co);

if rank_Co == n
    disp('The system is controllable. Pole placement is possible.');
else
    error('The system is not controllable. Pole placement is not possible.');
end

%% 6. Desired closed-loop poles

% Open-loop poles are approximately -1 +/- 3j.
% We choose faster real stable poles.

desired_poles = [-4 -5];

disp('Desired closed-loop poles:');
disp(desired_poles);

%% 7. State feedback gain using place

K = place(A, B, desired_poles);

disp('State feedback gain K:');
disp(K);

%% 8. Closed-loop matrix

Acl = A - B*K;

disp('Closed-loop matrix Acl = A - B*K:');
disp(Acl);

closed_loop_poles = eig(Acl);

disp('Closed-loop poles:');
disp(closed_loop_poles);

%% 9. Precompensation gain for reference tracking

% Closed-loop system:
% x_dot = (A-BK)x + B*Nbar*r
% y = Cx
%
% Nbar is used so that the steady-state output follows a unit step reference.

Nbar = -1 / (C * inv(Acl) * B);

disp('Precompensation gain Nbar:');
disp(Nbar);

Bcl = B * Nbar;

sys_closed = ss(Acl, Bcl, C, D);

%% 10. Step response comparison

t = 0:0.01:8;

figure;
step(sys_open, t);
hold on;
step(sys_closed, t);
grid on;
legend('Open-loop response', 'Closed-loop response');
title('TP4 - Open-loop and Closed-loop Step Responses');
xlabel('Time (s)');
ylabel('Displacement x(t)');

%% 11. Initial condition response comparison

x0 = [1;
      0];

sys_open_initial = ss(A, zeros(2,1), C, 0);
sys_closed_initial = ss(Acl, zeros(2,1), C, 0);

figure;
initial(sys_open_initial, x0, t);
hold on;
initial(sys_closed_initial, x0, t);
grid on;
legend('Open-loop initial response', 'Closed-loop initial response');
title('TP4 - Initial Condition Response Comparison');
xlabel('Time (s)');
ylabel('Displacement x(t)');

%% 12. State response and control signal

% To compute the states, define a system whose output is the full state vector.

sys_states_closed = ss(Acl, Bcl, eye(2), zeros(2,1));

r = ones(size(t));       % Unit step reference

[x_states, t_states] = lsim(sys_states_closed, r, t);

x1 = x_states(:,1);
x2 = x_states(:,2);

% Control law: u = -Kx + Nbar*r
u = -K * x_states' + Nbar * r;
u = u';

figure;
plot(t_states, x1, 'LineWidth', 1.5);
hold on;
plot(t_states, x2, 'LineWidth', 1.5);
grid on;
legend('State x1: position', 'State x2: velocity');
title('TP4 - Closed-loop State Evolution');
xlabel('Time (s)');
ylabel('States');

figure;
plot(t_states, u, 'LineWidth', 1.5);
grid on;
title('TP4 - Control Signal u(t)');
xlabel('Time (s)');
ylabel('Control force u(t)');

%% 13. Save all figures

figHandles = findall(0, 'Type', 'figure');

for i = 1:length(figHandles)
    fig = figHandles(i);
    figure(fig);

    filename_png = fullfile(outputFolder, sprintf('TP4_SISO_Figure_%d.png', i));
    saveas(fig, filename_png);

    filename_fig = fullfile(outputFolder, sprintf('TP4_SISO_Figure_%d.fig', i));
    savefig(fig, filename_fig);
end

disp('All TP4 figures have been saved.');

%% 14. Save numerical results in a text file

resultsFile = fullfile(outputFolder, 'TP4_SISO_results.txt');

diary(resultsFile);

disp('==============================================');
disp('TP4 - State Feedback Control of a SISO System');
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

disp('Open-loop poles:');
disp(open_loop_poles);

disp('Controllability matrix:');
disp(Co);

fprintf('Rank of controllability matrix = %d\n', rank_Co);

disp('Desired closed-loop poles:');
disp(desired_poles);

disp('State feedback gain K:');
disp(K);

disp('Closed-loop matrix Acl:');
disp(Acl);

disp('Closed-loop poles:');
disp(closed_loop_poles);

disp('Precompensation gain Nbar:');
disp(Nbar);

diary off;

%% 15. Save MATLAB workspace

save(fullfile(outputFolder, 'TP4_SISO_workspace.mat'));

disp('TP4 SISO results have been saved successfully.');
disp(['Results folder: ', outputFolder]);