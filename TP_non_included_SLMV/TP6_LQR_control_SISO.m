%% TP6 - LQR Control for a SISO System
% Module: Linear Multivariable Systems
% System: Mass-Spring-Damper
% Objective:
% Design an LQR controller and compare it with pole placement control.

clear;
clc;
close all;

%% 1. Save path

mainFolder = 'D:\S3 MTA\7- SLMV & SED & Commande Machine\SLMV\TPs';
outputFolder = fullfile(mainFolder, 'TP6_SISO_results');

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

open_loop_poles = pole(sys_open);

disp('Open-loop poles:');
disp(open_loop_poles);

%% 5. Check controllability

Co = ctrb(A, B);
rank_Co = rank(Co);
n = size(A,1);

fprintf('Rank of controllability matrix = %d\n', rank_Co);

if rank_Co == n
    disp('The system is controllable. LQR control is possible.');
else
    error('The system is not controllable. LQR control is not possible.');
end

%% 6. LQR design

% Q penalizes the states.
% R penalizes the control effort.
%
% A high value on Q(1,1) means that position error is strongly penalized.

Q = [100 0;
     0   1];

R = 1;

[K_lqr, S_lqr, poles_lqr] = lqr(A, B, Q, R);

disp('LQR gain K_lqr:');
disp(K_lqr);

disp('Closed-loop poles with LQR:');
disp(poles_lqr);

%% 7. Closed-loop LQR system

Acl_lqr = A - B*K_lqr;

% Precompensation gain for reference tracking
Nbar_lqr = -1 / (C * (Acl_lqr \ B));

Bcl_lqr = B * Nbar_lqr;

sys_lqr = ss(Acl_lqr, Bcl_lqr, C, D);

disp('LQR closed-loop matrix Acl_lqr:');
disp(Acl_lqr);

disp('LQR precompensation gain Nbar_lqr:');
disp(Nbar_lqr);

%% 8. Pole placement controller for comparison

desired_poles_pp = [-4 -5];

K_pp = place(A, B, desired_poles_pp);

Acl_pp = A - B*K_pp;

Nbar_pp = -1 / (C * (Acl_pp \ B));

Bcl_pp = B * Nbar_pp;

sys_pp = ss(Acl_pp, Bcl_pp, C, D);

poles_pp = eig(Acl_pp);

disp('Pole placement gain K_pp:');
disp(K_pp);

disp('Closed-loop poles with pole placement:');
disp(poles_pp);

%% 9. Step response comparison

t = 0:0.01:8;

figure;
step(sys_open, t);
hold on;
step(sys_pp, t);
step(sys_lqr, t);
grid on;
legend('Open-loop', 'Pole placement', 'LQR');
title('TP6 - Step Response Comparison');
xlabel('Time (s)');
ylabel('Displacement x(t)');

%% 10. State evolution with LQR

sys_states_lqr = ss(Acl_lqr, Bcl_lqr, eye(2), zeros(2,1));

r = ones(size(t));

[x_lqr, t_lqr] = lsim(sys_states_lqr, r, t);

x1_lqr = x_lqr(:,1);
x2_lqr = x_lqr(:,2);

u_lqr = -K_lqr * x_lqr' + Nbar_lqr * r;
u_lqr = u_lqr';

figure;
plot(t_lqr, x1_lqr, 'LineWidth', 1.5);
hold on;
plot(t_lqr, x2_lqr, 'LineWidth', 1.5);
grid on;
legend('State x1: position', 'State x2: velocity');
title('TP6 - LQR State Evolution');
xlabel('Time (s)');
ylabel('States');

%% 11. Control signal comparison

sys_states_pp = ss(Acl_pp, Bcl_pp, eye(2), zeros(2,1));

[x_pp, t_pp] = lsim(sys_states_pp, r, t);

u_pp = -K_pp * x_pp' + Nbar_pp * r;
u_pp = u_pp';

figure;
plot(t_pp, u_pp, 'LineWidth', 1.5);
hold on;
plot(t_lqr, u_lqr, 'LineWidth', 1.5);
grid on;
legend('Pole placement control signal', 'LQR control signal');
title('TP6 - Control Signal Comparison');
xlabel('Time (s)');
ylabel('Control force u(t)');

%% 12. Pole-zero map comparison

figure;
pzmap(sys_open);
hold on;
pzmap(sys_pp);
pzmap(sys_lqr);
grid on;
legend('Open-loop', 'Pole placement', 'LQR');
title('TP6 - Pole-Zero Map Comparison');

%% 13. Save all figures

figHandles = findall(0, 'Type', 'figure');

for i = 1:length(figHandles)
    fig = figHandles(i);
    figure(fig);

    filename_png = fullfile(outputFolder, sprintf('TP6_SISO_Figure_%d.png', i));
    saveas(fig, filename_png);

    filename_fig = fullfile(outputFolder, sprintf('TP6_SISO_Figure_%d.fig', i));
    savefig(fig, filename_fig);
end

disp('All TP6 figures have been saved.');

%% 14. Save numerical results in a text file

resultsFile = fullfile(outputFolder, 'TP6_SISO_results.txt');

diary(resultsFile);

disp('==============================================');
disp('TP6 - LQR Control for a SISO System');
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

disp('LQR weighting matrix Q:');
disp(Q);

disp('LQR weighting matrix R:');
disp(R);

disp('LQR gain K_lqr:');
disp(K_lqr);

disp('LQR closed-loop poles:');
disp(poles_lqr);

disp('LQR closed-loop matrix Acl_lqr:');
disp(Acl_lqr);

disp('LQR precompensation gain Nbar_lqr:');
disp(Nbar_lqr);

disp('----------------------------------------------');
disp('Pole placement comparison');
disp('----------------------------------------------');

disp('Desired pole placement poles:');
disp(desired_poles_pp);

disp('Pole placement gain K_pp:');
disp(K_pp);

disp('Pole placement closed-loop poles:');
disp(poles_pp);

disp('Pole placement precompensation gain Nbar_pp:');
disp(Nbar_pp);

diary off;

%% 15. Save MATLAB workspace

save(fullfile(outputFolder, 'TP6_SISO_workspace.mat'));

disp('TP6 SISO results have been saved successfully.');
disp(['Results folder: ', outputFolder]);