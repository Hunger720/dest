/**
    This file is part of Deformable Shape Tracking (DEST).

    Copyright(C) 2015/2016 Christoph Heindl
    All rights reserved.

    This software may be modified and distributed under the terms
    of the BSD license.See the LICENSE file for details.
*/

#include <dest/dest.h>

#include <dest/face/face_detector.h>
#include <dest/util/draw.h>
#include <dest/util/convert.h>
#include <dest/util/glob.h>
#include <random>
#include <opencv2/opencv.hpp>
#include <tclap/CmdLine.h>

#include <windows.h>
#include <output.h>

/**
    Sample program to predict shape landmarks on images.

    This program takes an image, a learnt tracker and an OpenCV face detector to
    compute shape landmark positions. OpenCV face detection works based on Viola
    Jones algorithm which requires a training phase as well. Suitable classifier 
    files can be donwloaded from OpenCV or from the dest/etc directory.

    Use any key to cycle throgh tracker cascades and see incremetal updates. Start
    configuration of shape landmarks is given in red.

    Note that his example uses an OpenCV face detector based on Viola Jones to
    provide the initial face rectangle. Therefore, you should only trackers that
    have been trained on the same input.

*/
int main(int argc, char **argv)
{
    struct {
        std::string detector;
        std::string tracker;
        std::string image;
    } opts;

    try {
        TCLAP::CmdLine cmd("Test regressor on a single image.", ' ', "0.9");
        TCLAP::ValueArg<std::string> trackerArg("t", "tracker", "Trained tracler to load", true, "dest.bin", "file");
        TCLAP::ValueArg<std::string> detectorArg("d", "detector", "OpenCV face detector to load", true, "cascade.xml", "string");        
        TCLAP::UnlabeledValueArg<std::string> imageArg("image", "Image to align or directory of images.", true, "img.png", "file");

        cmd.add(&detectorArg);
        cmd.add(&trackerArg);
        cmd.add(&imageArg);

        cmd.parse(argc, argv);

        opts.detector = detectorArg.getValue();
        opts.tracker = trackerArg.getValue();
        opts.image = imageArg.getValue();
    }
    catch (TCLAP::ArgException &e) {
        std::cerr << "Error: " << e.error() << " for arg " << e.argId() << std::endl;
        return -1;
    }
    
    std::vector<std::string> extensions;
    extensions.push_back("png");
    extensions.push_back("jpg");
    extensions.push_back("jpeg");
    extensions.push_back("bmp");
    std::vector<std::string> paths = dest::util::findFilesInDir(opts.image, extensions, false, true);
    
    if (paths.empty()) {
        paths.push_back(opts.image);
    }
    
    dest::face::FaceDetector fd;
    if (!fd.loadClassifiers(opts.detector)) {
        std::cout << "Failed to load classifiers." << std::endl;
        return 0;
    }
    
    dest::core::Tracker t;
    if (!t.load(opts.tracker)) {
        std::cout << "Failed to load tracker." << std::endl;
        return 0;
    }

    std::cout << "Found " << paths.size() << " images." << std::endl;
    bool done = false;
    for (size_t i = 0; i < paths.size() && !done; ++i) {
        
        cv::Mat imgCV = cv::imread(paths[i], cv::IMREAD_GRAYSCALE);
        if (imgCV.empty()) {
            std::cout << "Failed to load image " << paths[i] << std::endl;
            continue;
        }
        
        dest::core::Image img;
        dest::util::toDest(imgCV, img);
        
        dest::core::Rect r;
        if (!fd.detectSingleFace(img, r)) {
            std::cout << "Failed to detect face" << std::endl;
            continue;
        }
        
        // Default inverse shape normalization. Needs to be equivalent to training.
        dest::core::ShapeTransform shapeToImage = dest::core::estimateSimilarityTransform(dest::core::unitRectangle(), r);
        
        std::vector<dest::core::Shape> steps;
		DWORD start = GetTickCount();
        dest::core::Shape s = t.predict(img, shapeToImage, &steps);
		DWORD end = GetTickCount();
		std::cout << "face alignment time: " << (end-start)*1.0/1000 << " s\n";

		const char* path = "C:/Users/Hunger/Documents/MATLAB/3DMM_edges/testImages/180618/render_face_194landmarks.txt";
        
        size_t id = 0;
        while (id < steps.size()) {
            
            cv::Scalar color = (id == steps.size() - 1) ? cv::Scalar(0, 255, 0) : cv::Scalar(255, 255, 255);
            cv::Mat tmp = dest::util::drawShape(img, steps[id], color);
            cv::imshow("prediction", tmp);
            
            id = (id + 1);
            
            int key = cv::waitKey();
            
			if (key == 'x')
                done = true;

			if (key == 's')
			{
				float* landmark = new float[s.cols() * 2];
				for (int i = 0; i < s.cols(); ++i)
				{
					landmark[i * 2 + 0] = s(0, i);
					landmark[i * 2 + 1] = s(1, i);
				}
				shape_output(path, landmark, s.cols());
			}
        }


    }
    
    

    
    return 0;
}
