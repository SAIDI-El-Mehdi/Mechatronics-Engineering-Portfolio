%% TP7 - Frequency Response of a SISO System
% Module: Linear Multivariable Systems
% System: Mass-Spring-Damper
% Objective:
% Study the frequency response using Bode plot, Nyquist plot,
% gain margin, phase margin and bandwidth.

clear;
clc;
close all;

%% 1. Save path

mainFolder = 'D:\S3 MTA\7- SLMV & SED & Commande Machine\SLMV\TPs';
outputFolder = fullfile(mainFolder, 'TP7_SISO_results');

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

%% 6. Static gain

G0 = dcgain(sys);

disp('Static gain G(0):');
disp(G0);

%% 7. Poles

poles_sys = pole(sys);

disp('System poles:');
disp(poles_sys);

%% 8. Bode plot

figure;
bode(sys);
grid on;
title('TP7 - Bode Diagram of the SISO System');

%% 9. Bode plot with stability margins

figure;
margin(sys);
grid on;
title('TP7 - Bode Diagram with Gain and Phase Margins');

%% 10. Nyquist plot

figure;
nyquist(sys);
grid on;
title('TP7 - Nyquist Plot of the SISO System');

%% 11. Magnitude response only

figure;
bodemag(sys);
grid on;
title('TP7 - Magnitude Response of the SISO System');

%% 12. Compute gain margin, phase margin and crossover frequencies

[Gm, Pm, Wcg, Wcp] = margin(sys);

disp('Gain margin Gm:');
disp(Gm);

disp('Phase margin Pm:');
disp(Pm);

disp('Gain crossover frequency Wcg:');
disp(Wcg);

disp('Phase crossover frequency Wcp:');
disp(Wcp);

%% 13. Bandwidth

BW = bandwidth(sys);

disp('Bandwidth of the system:');
disp(BW);

%% 14. Frequency response data

w = logspace(-1, 2, 500);     % Frequency range from 0.1 to 100 rad/s

[mag, phase, wout] = bode(sys, w);

mag = squeeze(mag);
phase = squeeze(phase);

mag_dB = 20*log10(mag);

%% 15. Manual magnitude plot

figure;
semilogx(wout, mag_dB, 'LineWidth', 1.5);
grid on;
title('TP7 - Magnitude Plot');
xlabel('Frequency (rad/s)');
ylabel('Magnitude (dB)');

%% 16. Manual phase plot

figure;
semilogx(wout, phase, 'LineWidth', 1.5);
grid on;
title('TP7 - Phase Plot');
xlabel('Frequency (rad/s)');
ylabel('Phase (degrees)');

%% 17. Save all figures

figHandles = findall(0, 'Type', 'figure');

for i = 1:length(figHandles)
    fig = figHandles(i);
    figure(fig);

    filename_png = fullfile(outputFolder, sprintf('TP7_SISO_Figure_%d.png', i));
    saveas(fig, filename_png);

    filename_fig = fullfile(outputFolder, sprintf('TP7_SISO_Figure_%d.fig', i));
    savefig(fig, filename_fig);
end

disp('All TP7 figures have been saved.');

%% 18. Save numerical results in a text file

resultsFile = fullfile(outputFolder, 'TP7_SISO_results.txt');

diary(resultsFile);

disp('==============================================');
disp('TP7 - Frequency Response of a SISO System');
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

disp('Static gain G(0):');
disp(G0);

disp('System poles:');
disp(poles_sys);

disp('Frequency response results:');

disp('Gain margin Gm:');
disp(Gm);

disp('Phase margin Pm:');
disp(Pm);

disp('Gain crossover frequency Wcg:');
disp(Wcg);

disp('Phase crossover frequency Wcp:');
disp(Wcp);

disp('Bandwidth:');
disp(BW);

if isinf(Gm)
    disp('Interpretation: The gain margin is infinite because no critical gain crossing is detected.');
end

if isnan(Pm) || isinf(Pm)
    disp('Interpretation: The phase margin is not finite because the system has no 0 dB gain crossover.');
end

diary off;

%% 19. Save MATLAB workspace

save(fullfile(outputFolder, 'TP7_SISO_workspace.mat'));

disp('TP7 SISO results have been saved successfully.');
disp(['Results folder: ', outputFolder]);