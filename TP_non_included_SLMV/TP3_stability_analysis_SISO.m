%% TP3 - Stability Analysis of a SISO System
% Module: Linear Multivariable Systems
% System: Mass-Spring-Damper
% Objective:
% Analyze stability using eigenvalues, poles, step response,
% impulse response and pole-zero map.

clear;
clc;
close all;

%% 1. Save path

mainFolder = 'D:\S3 MTA\7- SLMV & SED & Commande Machine\SLMV\TPs';
outputFolder = fullfile(mainFolder, 'TP3_SISO_results');

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

%% 5. Transfer function

G = tf(sys);

disp('Transfer function G(s):');
G

%% 6. Eigenvalues and poles

eig_A = eig(A);
poles_sys = pole(sys);

disp('Eigenvalues of matrix A:');
disp(eig_A);

disp('Poles of the system:');
disp(poles_sys);

%% 7. Stability test

real_parts = real(eig_A);

if all(real_parts < 0)
    stability_result = 'The system is stable.';
elseif any(real_parts > 0)
    stability_result = 'The system is unstable.';
else
    stability_result = 'The system is marginally stable.';
end

disp('Stability conclusion:');
disp(stability_result);

%% 8. Step response

figure;
step(sys);
grid on;
title('TP3 - Step Response of the SISO System');
xlabel('Time');
ylabel('Displacement x(t)');

%% 9. Impulse response

figure;
impulse(sys);
grid on;
title('TP3 - Impulse Response of the SISO System');
xlabel('Time');
ylabel('Displacement x(t)');

%% 10. Pole-zero map

figure;
pzmap(sys);
grid on;
title('TP3 - Pole-Zero Map of the SISO System');

%% 11. Comparison between stable, unstable and marginal systems

% Stable example
A_stable = [0 1;
           -10 -2];

% Unstable example
A_unstable = [0 1;
              10 2];

% Marginally stable example
A_marginal = [0 1;
             -10 0];

B_ex = [0;
        1];

C_ex = [1 0];

D_ex = 0;

sys_stable = ss(A_stable, B_ex, C_ex, D_ex);
sys_unstable = ss(A_unstable, B_ex, C_ex, D_ex);
sys_marginal = ss(A_marginal, B_ex, C_ex, D_ex);

figure;
step(sys_stable, sys_unstable, sys_marginal);
grid on;
legend('Stable system', 'Unstable system', 'Marginally stable system');
title('TP3 - Comparison of Stability Behaviors');
xlabel('Time');
ylabel('Output');

%% 12. Save all figures

figHandles = findall(0, 'Type', 'figure');

for i = 1:length(figHandles)
    fig = figHandles(i);
    figure(fig);

    filename_png = fullfile(outputFolder, sprintf('TP3_SISO_Figure_%d.png', i));
    saveas(fig, filename_png);

    filename_fig = fullfile(outputFolder, sprintf('TP3_SISO_Figure_%d.fig', i));
    savefig(fig, filename_fig);
end

disp('All TP3 figures have been saved.');

%% 13. Save numerical results in a text file

resultsFile = fullfile(outputFolder, 'TP3_SISO_results.txt');

diary(resultsFile);

disp('==============================================');
disp('TP3 - Stability Analysis of a SISO System');
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

disp('Transfer function G(s):');
G

disp('Eigenvalues of matrix A:');
disp(eig_A);

disp('Poles of the system:');
disp(poles_sys);

disp('Real parts of eigenvalues:');
disp(real_parts);

disp('Stability conclusion:');
disp(stability_result);

disp('----------------------------------------------');
disp('Comparison systems');
disp('----------------------------------------------');

disp('Eigenvalues of stable example:');
disp(eig(A_stable));

disp('Eigenvalues of unstable example:');
disp(eig(A_unstable));

disp('Eigenvalues of marginally stable example:');
disp(eig(A_marginal));

diary off;

%% 14. Save MATLAB workspace

save(fullfile(outputFolder, 'TP3_SISO_workspace.mat'));

disp('TP3 SISO results have been saved successfully.');
disp(['Results folder: ', outputFolder]);