for i = 0 : 206
    fn = num2str(i);
    filename = strcat('D:\data\OR_11_Nov_17\Patient\DICOM\IM.', fn);
    info = dicominfo(filename);
    im1 = double(dicomread(filename));
    im1 = im1 / max(max(im1));
    ii = autocontrast(uint8(im1*255));
    filenameOut = strcat('D:\data\OR_11_Nov_17\Patient\PNG\FILE', sprintf('%.4d', info.InstanceNumber), '.png');
    imshow(ii)
    imwrite(ii, filenameOut);
    pause(0.2)
end