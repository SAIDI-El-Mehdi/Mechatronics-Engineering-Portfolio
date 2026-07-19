%% TP2 - Controllability and Observability of a SISO System
% Module: Linear Multivariable Systems
% System: Mass-Spring-Damper
% Objective:
% Study controllability and observability using ctrb, obsv and rank.

clear;
clc;
close all;

%% 1. Save path

mainFolder = 'D:\S3 MTA\7- SLMV & SED & Commande Machine\SLMV\TPs';
outputFolder = fullfile(mainFolder, 'TP2_SISO_results');

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

%% 5. Number of states

n = size(A,1);

fprintf('Number of states = %d\n', n);

%% 6. Controllability analysis

Co = ctrb(A, B);
rank_Co = rank(Co);

disp('Controllability matrix Co:');
disp(Co);

fprintf('Rank of controllability matrix = %d\n', rank_Co);

if rank_Co == n
    controllability_result = 'The system is controllable.';
else
    controllability_result = 'The system is NOT controllable.';
end

disp(controllability_result);

%% 7. Observability analysis

Ob = obsv(A, C);
rank_Ob = rank(Ob);

disp('Observability matrix Ob:');
disp(Ob);

fprintf('Rank of observability matrix = %d\n', rank_Ob);

if rank_Ob == n
    observability_result = 'The system is observable.';
else
    observability_result = 'The system is NOT observable.';
end

disp(observability_result);

%% 8. Plot rank comparison

figure;

bar([rank_Co rank_Ob n]);
grid on;

set(gca, 'XTickLabel', {'Rank Co', 'Rank Ob', 'Number of states'});
ylabel('Rank value');
title('TP2 - Controllability and Observability Rank Test');

%% 9. Save all figures

figHandles = findall(0, 'Type', 'figure');

for i = 1:length(figHandles)
    fig = figHandles(i);
    figure(fig);

    filename_png = fullfile(outputFolder, sprintf('TP2_SISO_Figure_%d.png', i));
    saveas(fig, filename_png);

    filename_fig = fullfile(outputFolder, sprintf('TP2_SISO_Figure_%d.fig', i));
    savefig(fig, filename_fig);
end

disp('All TP2 figures have been saved.');

%% 10. Save numerical results in a text file

resultsFile = fullfile(outputFolder, 'TP2_SISO_results.txt');

diary(resultsFile);

disp('==============================================');
disp('TP2 - Controllability and Observability of a SISO System');
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

fprintf('Number of states = %d\n', n);

disp('----------------------------------------------');
disp('Controllability analysis');
disp('----------------------------------------------');

disp('Controllability matrix Co:');
disp(Co);

fprintf('Rank of controllability matrix = %d\n', rank_Co);
disp(controllability_result);

disp('----------------------------------------------');
disp('Observability analysis');
disp('----------------------------------------------');

disp('Observability matrix Ob:');
disp(Ob);

fprintf('Rank of observability matrix = %d\n', rank_Ob);
disp(observability_result);

diary off;

%% 11. Save MATLAB workspace

save(fullfile(outputFolder, 'TP2_SISO_workspace.mat'));

disp('TP2 SISO results have been saved successfully.');
disp(['Results folder: ', outputFolder]);