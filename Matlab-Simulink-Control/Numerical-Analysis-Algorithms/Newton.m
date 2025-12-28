function [x, niter] = newton1ln()
    % Initialisation
    x0 = 0; % Valeur initiale
    epsilon = 1e-10; % Tolérance
    maxIter = 1000; % Nombre maximal d'itérations
    
    % Définir la fonction et sa dérivée
    f = @(x) exp(-2*x) - cos(x) - 3; % Fonction
    df = @(x) -2*exp(-2*x) + sin(x); % Dérivée de la fonction
    
    % Algorithme de Newton-Raphson
    x = x0;
    niter = 0;
    while niter < maxIter
        niter = niter + 1;
        x_new = x - f(x) / df(x); % Formule de Newton-Raphson
        
        % Vérifier la condition d'arrêt
        if abs(x_new - x) < epsilon
            x = x_new;
            fprintf('La solution trouvée est x = %.10f après %d itérations.\n', x, niter);
            return;
        end
        
        x = x_new; % Mettre à jour x
    end
    
    % Si le maximum d'itérations est atteint
    fprintf('La méthode de Newton n''a pas convergé après %d itérations.\n', maxIter);
end
