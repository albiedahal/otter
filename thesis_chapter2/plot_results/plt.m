%%% The script needs polishing. Won't generate the figures in this
%%% direcotry.

clear all; clc; close all;


load('abqc.mat');
load('mosc.mat');
load('trelteun_abq.mat');
load('trelteun_mos.mat');
load('trelteda_abq.mat');
load('trelteda_mos.mat');
load('trplteun_abq.mat');
load('trplteun_mos.mat');
load('trplteda_abq.mat');
load('trplteda_mos.mat');
load('trelbeun_abq.mat');
load('trelbeun_mos.mat');
load('trelbeda_abq.mat');
load('trelbeda_mos.mat');
load('trplbeun_abq.mat');
load('trplbeun_mos.mat');
load('trplbeda_abq.mat');
load('trplbeda_mos.mat');



ax = gca;
ax.XAxisLocation = 'origin';
ax.YAxisLocation = 'origin';


% 
figure;
plot(abqc(:,2),abqc(:,5),'-k')
hold on;
plot(mosc(:,4),mosc(:,5),'--g')
xlim([-2.5e-2 2.5e-2]);
ylim([-0.8 0.8]);
legend('Abaqus','Moose')
grid on;


figure;
plot(trelteun_abq(:,1),trelteun_abq(:,11),'--b')
hold on;
plot(trelteun_mos(:,1),trelteun_mos(:,2),':k','Linewidth',1)
xlim([0 3]);
ylim([-0.4 0.4]);
grid on;

figure;
plot(trelteda_abq(:,1),trelteda_abq(:,11),'--b')
hold on;
plot(trelteda_mos(:,1),trelteda_mos(:,2),':k')

figure;
plot(trplbeun_abq(:,1),trplbeun_abq(:,6),'--b')
hold on;
plot(trplbeun_mos(:,1),-trplbeun_mos(:,2),':k','Linewidth',1)


figure;
plot(trplbeda_abq(:,1),trplbeda_abq(:,6),'--b')
hold on;
plot(trplbeda_mos(:,1),-trplbeda_mos(:,2),':k','Linewidth',1)
xlim([0 32]);
ylim([-1.5 1.5]);
grid on;

figure;
plot(trelbeun_abq(:,1),trelbeun_abq(:,2),'--b')
hold on;
plot(trelbeun_mos(:,1),-trelbeun_mos(:,2),':k','Linewidth',1)


figure;
plot(trelbeda_abq(:,1),trelbeda_abq(:,2),'--b')
hold on;
plot(trelbeda_mos(:,1),-trelbeda_mos(:,2),':k','Linewidth',1)
xlim([0 32]);
ylim([-1.2 1.2]);
grid on;


figure;
plot(trplteda_abq(:,1),trplteda_abq(:,6),'--b')
hold on;
plot(trplteda_mos(:,1),trplteda_mos(:,2),':k','Linewidth',1)
xlim([0 3]);
ylim([-0.4 0.4]);

figure;
plot(trplteun_abq(:,1),trplteun_abq(:,6),'--b')
hold on;
plot(trplteun_mos(:,1),trplteun_mos(:,2),':k','Linewidth',1)
xlim([0 3]);
ylim([-0.4 0.4]);

figure;
plot(elcplbeun_abq(:,1),elcplbeun_abq(:,6),'--b')
hold on;
plot(elcplbeun_mos(:,1),elcplbeun_mos(:,2),':k')
xlim([0 3]);
ylim([-0.4 0.4]);
grid on;



figure;
plot(elcplbeda_abq(:,1),elcplbeda_abq(:,6),'--b')
hold on;
plot(elcplbeda_mos(:,1),elcplbeda_mos(:,2),':k')

grid on;
% 
% figure;
% plot(trelteun_abq(:,1),trelteun_abq(:,7))
% hold on;
% plot(trelteun_mos(:,1),trelteun_mos(:,8))
% % xlim([-2e-2 2e-2]);
% % ylim([-0.7 0.7]);
% grid on;
% 
figure;
plot(trelteda_abq(:,1),trelteda_abq(:,11),'--b')
hold on;
plot(trelteda_mos(:,1),trelteda_mos(:,2),':k')
% xlim([0 3]);
% ylim([-0.35 0.35]);
grid on;
% 
% figure;
% plot(trelteda_abq(:,1),trelteda_abq(:,5))
% hold on;
% plot(trelteda_mos(:,1),trelteda_mos(:,6))
% % xlim([-2e-2 2e-2]);
% % ylim([-0.7 0.7]);
% grid on;


