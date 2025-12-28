%% 1. Create the Map (Environment)
% خريطة فيها 50x50 متر
map = binaryOccupancyMap(50, 50, 1);

% لائحة العقبات: [x, y, width, height]
obstacles = [
    10 0 2 20;   % Wall 1
    30 30 2 20;  % Wall 2
    20 15 10 2;  % Wall 3 (الحيط الوسطاني)
    0 49 50 1;   % Top border
    0 0 50 1;    % Bottom border
    0 0 1 50;    % Left border
    49 0 1 50    % Right border
];

% إضافة جميع العقبات للخريطة (Boucle Smart)
for i = 1:size(obstacles, 1)
    x_start = obstacles(i, 1);
    y_start = obstacles(i, 2);
    w = obstacles(i, 3);
    h = obstacles(i, 4);

    % توليد نقاط الشبكة لكل حيط
    [x_pts, y_pts] = meshgrid(x_start:0.5:(x_start+w), ...
                              y_start:0.5:(y_start+h));

    % وضع العقبات في الخريطة
    setOccupancy(map, [x_pts(:) y_pts(:)], 1);
end

%% 2. Configure the Planner (The Brain)
% تعريف فضاء الحالة (x, y, theta)
ss = stateSpaceSE2;
ss.StateBounds = [map.XWorldLimits; map.YWorldLimits; [-pi pi]];

% إعداد المدقق (Validator)
validator = validatorOccupancyMap(ss);
validator.Map = map;
validator.ValidationDistance = 0.5;

% إعداد المخطط Hybrid A*
planner = plannerHybridAStar(validator, ...
    'MinTurningRadius', 4.0, ...       % شعاع الدوران المناسب للسيارة
    'MotionPrimitiveLength', 6);

%% 3. Define Start and Goal
% [x, y, theta]
startPose = [5 5 0];         % البداية
goalPose  = [45 45 pi/2];    % النهاية

%% 4. Plan the Path
disp('Planning path... Please wait.');
refPath = plan(planner, startPose, goalPose);   % Hybrid A* [web:198]

%% 5. Visualization of Planned Path
figure('Name', 'Path Planning using Hybrid A*');
show(map); hold on;
show(planner);
title('Hybrid A* Path with Obstacles');

if ~isempty(refPath)
    plot(refPath.States(:,1), refPath.States(:,2), 'g-', 'LineWidth', 2);
    legend('Map', 'Search Tree', 'Start', 'Goal', 'Planned Path');
    disp('Path found successfully!');
else
    disp('Failed to find a path!');
end
hold off;

%% 6. Save Data for Simulink
if ~isempty(refPath)
    refPose = refPath.States; % [x, y, theta]
    save('path_data.mat', 'refPose');
    disp('Data saved to path_data.mat');
end

%% 7. Run Simulink Model and Plot Final Result
% تشغيل نموذج AEB في Simulink واسترجاع النتائج [web:201]
out = sim('AEB_Project');           % يستعمل Stop Time اللي في Simulink

x_sim = out.sim_x(:,1);
y_sim = out.sim_y(:,1);

% تحميل المسار المخطط
load('path_data.mat');   % refPose

% الرسم النهائي للمقارنة
figure('Name','Final Project Result');
plot(refPose(:,1), refPose(:,2), 'g--','LineWidth',2);   % Planned Path
hold on;
plot(x_sim, y_sim, 'b-','LineWidth',2);                  % Actual Path
legend('Planned Path','Actual Path (Pure Pursuit)');
xlabel('X (m)'); ylabel('Y (m)');
grid on;
title('Autonomous Vehicle Path Tracking');
