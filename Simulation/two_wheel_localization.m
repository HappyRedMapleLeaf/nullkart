x = [1, 5, 8, 9];
y = [6, 2, 6, 11];
theta = [0, pi/4, pi/2, pi];

dL = [3*pi/4, 6*pi/4, 9*pi/4, 12*pi/4];
dR = [-3*pi/4, -6*pi/4, -9*pi/4, -12*pi/4];
w = 3;

dtheta = (dR - dL)/w

if dtheta == 0
    dx = (dR + dL)/2 .* cos(theta);
    dy = (dR + dL)/2 .* sin(theta);
else
    r = (w/2)*((dR+dL)./(dL-dR))

    dx = r.*(-sin(theta+dtheta) + sin(theta));
    dy = r.*( cos(theta+dtheta) - cos(theta));
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

to_plot_x2 = [x + dx]
to_plot_y2 = [y + dy]

u_2 = arrow_length * cos(theta+dtheta);
v_2 = arrow_length * sin(theta+dtheta);
scatter(to_plot_x2, to_plot_y2, 50, 'filled', 'MarkerFaceColor', 'green');
quiver(to_plot_x2, to_plot_y2, u_2, v_2, 'g', 'LineWidth', 0.5, 'MaxHeadSize', 0.1, 'AutoScale', 'off');

axis equal;
grid on;
waitfor(gcf);