#include "opencv2/opencv.hpp"
#include <stdlib.h>
#include <string>

using namespace cv;
using namespace std;

// Low Range BGR
int lr_bgr[3] = {19, 72, 111};
// High Range BGR
int hr_bgr[3] = {70, 213, 221};

int erosion_size = 50;
int dilation_size = 50;
int canny_thresh = 50;

Point dbg_tl, dbg_br;

void setup_color_trackbars(void)
{
    createTrackbar("inRange Low Scalar: Blue", "BGR Frame", &lr_bgr[0], 255);
    createTrackbar("inRange Low Scalar: Green", "BGR Frame", &lr_bgr[1], 255);
    createTrackbar("inRange Low Scalar: Red", "BGR Frame", &lr_bgr[2], 255);

    createTrackbar("inRange High Scalar: Blue", "BGR Frame", &hr_bgr[0], 255);
    createTrackbar("inRange High Scalar: Green", "BGR Frame", &hr_bgr[1], 255);
    createTrackbar("inRange High Scalar: Red", "BGR Frame", &hr_bgr[2], 255);

    createTrackbar("Erosion Size", "BGR Frame", &erosion_size, 50);
    createTrackbar("Dilation Size", "BGR Frame", &dilation_size, 50);

    createTrackbar("Canny Thresh", "BGR Frame", &canny_thresh, 100);
}

void create_position_trackbar(Point *tl, Point *br)
{
    createTrackbar("Top Left X:", "Main Frame", &tl->x, 400);
    createTrackbar("Top Left Y:", "Main Frame", &tl->y, 400);

    createTrackbar("Bottom Right X:", "Main Frame", &br->x, 400);
    createTrackbar("Bottom Right Y:", "Main Frame", &br->y, 400);
}

vector<Rect> create_pokemon_frame(int card_width, int card_length, Point tl_point)
{
    double ratio_x = ((double) card_width)/277.0;
    double ratio_y = ((double) card_length)/200.0;
    vector<Rect> bounding_boxes;

    // create_position_trackbar(&dbg_tl, &dbg_br);

    // Placement rectangle code
    // Rect first_edition = Rect(Point(tl_point.x + (dbg_tl.x * ratio_x), tl_point.y + (dbg_tl.y * ratio_y)),
    //                           Point(tl_point.x + (dbg_br.x * ratio_x), tl_point.y + (dbg_br.y * ratio_y)));

    //Pokemon Name
    Rect name_frame = Rect(Point(tl_point.x + (30.0 * ratio_x), tl_point.y + (16.0 * ratio_y)),
                           Point(tl_point.x + (173.0 * ratio_x), tl_point.y + (25.0 * ratio_y)));
    bounding_boxes.push_back(name_frame);
 
    // Pokemon Type Icon
    Rect type_icon = Rect(Point(tl_point.x + (223.0 * ratio_x), tl_point.y + (12.0 * ratio_y)),
                          Point(tl_point.x + (248.0 * ratio_x), tl_point.y + (25.0 * ratio_y)));

    bounding_boxes.push_back(type_icon);

    // Pokemon Series Icon
    Rect series_icon = Rect(Point(tl_point.x + (232.0 * ratio_x), tl_point.y + (103.0 * ratio_y)),
                            Point(tl_point.x + (259.0 * ratio_x), tl_point.y + (114.0 * ratio_y)));

    bounding_boxes.push_back(series_icon);

    // Pokemon Series ID
    Rect series_id = Rect(Point(tl_point.x + (225.0 * ratio_x), tl_point.y + (186.0 * ratio_y)),
                          Point(tl_point.x + (260.0 * ratio_x), tl_point.y + (194.0 * ratio_y)));

    bounding_boxes.push_back(series_id);

    // Pokemon First Edition
    Rect first_edition = Rect(Point(tl_point.x + (16.0 * ratio_x), tl_point.y + (103.0 * ratio_y)),
                              Point(tl_point.x + (41.0 * ratio_x), tl_point.y + (116.0 * ratio_y)));

    bounding_boxes.push_back(first_edition);

    return bounding_boxes;
}

int main(int, char**)
{
    VideoCapture cap(0); // open the default camera
    if(!cap.isOpened())  // check if we succeeded
        return -1;

    Mat edges;

    Point tlb_offset, brb_offset;
    while(1)
    {
        namedWindow("Main Frame", CV_WINDOW_AUTOSIZE);
        namedWindow("BGR Frame", CV_WINDOW_AUTOSIZE);

        Mat frame;
        cap >> frame; // get a new frame from camera
        cvtColor(frame, edges, CV_BGR2HSV);
        Mat yellow_range;

        //setup_color_trackbars();

		Mat erode_element = getStructuringElement(0, Size(2*erosion_size + 1, 2*erosion_size + 1), Point(erosion_size, erosion_size));
		Mat dilate_element = getStructuringElement(0, Size(2*dilation_size + 1, 2*dilation_size + 1), Point(dilation_size, dilation_size));

        inRange(edges, Scalar(lr_bgr[0], lr_bgr[1], lr_bgr[2]), Scalar(hr_bgr[0], hr_bgr[1], hr_bgr[2]), yellow_range);

        dilate(yellow_range, yellow_range, dilate_element);
        erode(yellow_range, yellow_range, erode_element);
       
        Canny( yellow_range, yellow_range, canny_thresh, canny_thresh*2, 3);
        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        findContours( yellow_range, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0,0));

        drawContours(frame, contours, 0, Scalar(0,0,255), 2);
        Rect boundRect = boundingRect(Mat(contours[0]));

        Point top_left_point = boundRect.tl();
        Point bottom_right_point = boundRect.br();

        rectangle(frame, top_left_point, bottom_right_point, Scalar(0,255,0), 2, 8, 0);
        
        //create_position_trackbar(&tlb_offset, &brb_offset);

        int card_width = bottom_right_point.x - top_left_point.x;
        int card_length = bottom_right_point.y - top_left_point.y;

        vector<Rect> bound_frames = create_pokemon_frame(card_width, card_length, top_left_point);

        for(int idx = 0; idx < bound_frames.size(); idx++)
        {
            rectangle(frame, bound_frames[idx].tl(), bound_frames[idx].br(), Scalar(255,0,0), 2, 8, 0);
            Mat temp_mat = frame(bound_frames[idx]);
            String frame_title = "Image" + to_string(idx);
            if (!temp_mat.empty())
            {
                imshow(frame_title, temp_mat);
            }
        }

        cout<<"Top Left Coord:"<<boundRect.tl()<<"; Bottom Right Coord:"<<boundRect.br()<<endl;
        imshow("BGR Frame", yellow_range);
        imshow("Main Frame", frame);
        if(waitKey(100) >= 0);
    }
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}
