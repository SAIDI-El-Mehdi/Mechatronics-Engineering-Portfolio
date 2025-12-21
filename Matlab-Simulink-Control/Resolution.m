% Exercice 1:
%1. Definition de A et b
A=[1,4,-1,1;2,7,1,-2;1,4,-1,2;3,-10,-2,5];
b=[2;16;1;-15];
disp('La matrice A:');
disp(A);
disp('La vecteur b:');
disp(b);
%2. Calcul du determinant de A
determinant=det(A);
fprintf('\nle determinat de A est:%.2f\n',determinant);
%3.Resolution du systeme A.X=b
X=A\b;
disp('La solution du systeme A.X=b est:')
disp(X);
