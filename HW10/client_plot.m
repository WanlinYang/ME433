function data = client_plot(port)

if ~isempty(instrfind)
    fclose(instrfind);
    delete(instrfind);
end

s = serial(port);
fopen(s);

fprintf(s,'%c\n','r');
nsamples = fscanf(s,'%d');
for i=1:nsamples
    datatemp = fscanf(s,'%f  %f  %f  %f');
    data(i,:) = [datatemp(1),datatemp(2),datatemp(3),datatemp(4)];
end

plot(data(:,1))
hold on
plot(data(:,2))
plot(data(:,3))
plot(data(:,4))
legend('raw','MAF','IIR','FIR')
ylabel('z-acc');
xlabel('Sample times');  

end