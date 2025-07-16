% original state
x = [1, 5, 8, 9];
y = [6, 2, 6, 11];
theta = [0, pi/4, pi/2, pi];

% localizer parameters
k1 = 2.0 * pi * 0.021 / 1680;
w = 0.24;

% localizer data
vL = [320, 320, 320, 320];
vR = [280, 280, 280, 280];

% update parameter
dt_s = 0.1;

% update algorithm
dL = vL.*(dt_s*k1)
dR = vR.*(dt_s*k1)

d_theta = (dR - dL)/w;

cos_theta = cos(theta);
sin_theta = sin(theta);

if abs(d_theta) == 1e-8
    dx = (dL + dR)/2 .* cos_theta;
    dy = (dL + dR)/2 .* sin_theta;
else
    r =  (dL + dR) ./ (-2.0 * d_theta);

    r_d_theta = r .* d_theta;
    d_theta_half = d_theta / 2;
    k2 = 1 - (d_theta .* d_theta) / 6.0;

    dx = r_d_theta .* ( sin_theta .* d_theta_half - cos_theta .* k2);
    dy = r_d_theta .* (-cos_theta .* d_theta_half - sin_theta .* k2);
end

w2 = w/2;
lwheelx = x - w2.*sin(theta);
lwheely = y + w2.*cos(theta);
rwheelx = x + w2.*sin(theta);
rwheely = y - w2.*cos(theta);

to_plot_x = [x; lwheelx; rwheelx];
to_plot_y = [y; lwheely; rwheely];

figure;
scatter(to_plot_x, to_plot_y, 50, 'filled', 'MarkerFaceColor', 'blue');
hold on;

arrow_length = 3;
u = arrow_length * cos(theta);
v = arrow_length * sin(theta);

quiver(x, y, u, v, 'r', 'LineWidth', 0.5, 'MaxHeadSize', 0.1, 'AutoScale', 'off');

to_plot_x2 = [x + dx];
to_plot_y2 = [y + dy];

u_2 = arrow_length * cos(theta+d_theta);
v_2 = arrow_length * sin(theta+d_theta);
scatter(to_plot_x2, to_plot_y2, 50, 'filled', 'MarkerFaceColor', 'green');
quiver(to_plot_x2, to_plot_y2, u_2, v_2, 'g', 'LineWidth', 0.5, 'MaxHeadSize', 0.1, 'AutoScale', 'off');

axis equal;
grid on;
waitfor(gcf);