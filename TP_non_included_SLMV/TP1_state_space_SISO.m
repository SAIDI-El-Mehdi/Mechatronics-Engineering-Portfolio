%% TP1 - State-Space Representation of a SISO System
% Module: Linear Multivariable Systems
% System: Mass-Spring-Damper
% Objective:
% Define a SISO state-space system, convert it to transfer function,
% compute eigenvalues and poles, and plot the step response.

clear;
clc;
close all;

%% 1. Save path

mainFolder = 'D:\S3 MTA\7- SLMV & SED & Commande Machine\SLMV\TPs';
outputFolder = fullfile(mainFolder, 'TP1_SISO_results');

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

%% 4. Display matrices

disp('Matrix A:');
disp(A);

disp('Matrix B:');
disp(B);

disp('Matrix C:');
disp(C);

disp('Matrix D:');
disp(D);

%% 5. Number of states, inputs and outputs

n_states = size(A,1);
n_inputs = size(B,2);
n_outputs = size(C,1);

fprintf('Number of states  = %d\n', n_states);
fprintf('Number of inputs  = %d\n', n_inputs);
fprintf('Number of outputs = %d\n', n_outputs);

%% 6. Create state-space system

sys = ss(A, B, C, D);

disp('State-space system:');
sys

%% 7. Transfer function

G = tf(sys);

disp('Transfer function G(s):');
G

%% 8. Eigenvalues and poles

eig_A = eig(A);
poles_sys = pole(sys);

disp('Eigenvalues of A:');
disp(eig_A);

disp('Poles of the system:');
disp(poles_sys);

%% 9. Static gain

G0 = dcgain(sys);

disp('Static gain G(0):');
disp(G0);

%% 10. Step response

figure;
step(sys);
grid on;
title('TP1 - Step Response of the SISO System');
xlabel('Time');
ylabel('Displacement x(t)');

%% 11. Impulse response

figure;
impulse(sys);
grid on;
title('TP1 - Impulse Response of the SISO System');
xlabel('Time');
ylabel('Displacement x(t)');

%% 12. Pole-zero map

figure;
pzmap(sys);
grid on;
title('TP1 - Pole-Zero Map of the SISO System');

%% 13. Save all figures

figHandles = findall(0, 'Type', 'figure');

for i = 1:length(figHandles)
    fig = figHandles(i);
    figure(fig);

    filename_png = fullfile(outputFolder, sprintf('TP1_SISO_Figure_%d.png', i));
    saveas(fig, filename_png);

    filename_fig = fullfile(outputFolder, sprintf('TP1_SISO_Figure_%d.fig', i));
    savefig(fig, filename_fig);
end

disp('All TP1 figures have been saved.');

%% 14. Save numerical results in a text file

resultsFile = fullfile(outputFolder, 'TP1_SISO_results.txt');

diary(resultsFile);

disp('==============================================');
disp('TP1 - State-Space Representation of a SISO System');
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

fprintf('Number of states  = %d\n', n_states);
fprintf('Number of inputs  = %d\n', n_inputs);
fprintf('Number of outputs = %d\n', n_outputs);

disp('State-space system:');
sys

disp('Transfer function G(s):');
G

disp('Eigenvalues of A:');
disp(eig_A);

disp('Poles of the system:');
disp(poles_sys);

disp('Static gain G(0):');
disp(G0);

diary off;

%% 15. Save MATLAB workspace

save(fullfile(outputFolder, 'TP1_SISO_workspace.mat'));

disp('TP1 SISO results have been saved successfully.');
disp(['Results folder: ', outputFolder]);