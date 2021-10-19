/*
This is an implementation of a volume measuring algorithm using
the ai vision sensor "HUSKYLENS"
*/
#include "HUSKYLENS.h"
#include "SoftwareSerial.h"

HUSKYLENS huskylens;
SoftwareSerial mySerial(10, 11); // RX, TX
//HUSKYLENS green line >> Pin 10; blue line >> Pin 11
void printResult(HUSKYLENSResult result);

//Setup code for serial monitor and huskylens
void setup()
{
    Serial.begin(115200);
    mySerial.begin(9600);
    //error in reading huskylens or serial error
    while (!huskylens.begin(mySerial))
    {
        Serial.println(F("Begin failed!"));
        Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protocol Type>>Serial 9600)"));
        Serial.println(F("2.Please recheck the connection."));
        delay(100);
    }
}

//Vector for dynamic container
template <typename Data>
class Vector
{
    size_t d_size;     // Stores no. of actually stored objects
    size_t d_capacity; // Stores allocated capacity
    Data *d_data;      // Stores data
public:
    Vector() : d_size(0), d_capacity(0), d_data(0){}; // Default constructor
    Vector(Vector const &other) : d_size(other.d_size), d_capacity(other.d_capacity), d_data(0)
    {
        d_data = (Data *)malloc(d_capacity * sizeof(Data));
        memcpy(d_data, other.d_data, d_size * sizeof(Data));
    };                           // Copy constuctor
    ~Vector() { free(d_data); }; // Destructor
    Vector &operator=(Vector const &other)
    {
        free(d_data);
        d_size = other.d_size;
        d_capacity = other.d_capacity;
        d_data = (Data *)malloc(d_capacity * sizeof(Data));
        memcpy(d_data, other.d_data, d_size * sizeof(Data));
        return *this;
    }; // Needed for memory management

    //add a new value to the vector
    void push_back(Data const &x)
    {
        if (d_capacity == d_size)
            resize();
        d_data[d_size++] = x;
    };                                                                // Adds new value. If needed, allocates more space
    size_t size() const { return d_size; };                           // Size getter
    Data const &operator[](size_t idx) const { return d_data[idx]; }; // Const getter
    Data &operator[](size_t idx) { return d_data[idx]; };             // Changeable getter
private:
    void resize()
    {
        d_capacity = d_capacity ? d_capacity * 2 : 1;
        Data *newdata = (Data *)malloc(d_capacity * sizeof(Data));
        memcpy(newdata, d_data, d_size * sizeof(Data));
        free(d_data);
        d_data = newdata;
    }; // Allocates double the old space
};



//Vector to store widths
Vector<double> widths;
//Vector to store lengths
Vector<double> lengths;

//swap two ints
void swap(double *xp, double *yp)
{
    double temp = *xp;
    *xp = *yp;
    *yp = temp;
}

// A function to implement bubble sort
//worl indicates which vector is being referenced
void bubbleSort(int worl, int n)
{
    //worl 0 us for widths
    if (worl == 0)
    {
        int i, j;
        for (i = 0; i < n - 1; i++)

            // Last i elements are already in place
            for (j = 0; j < n - i - 1; j++)
                if (widths[j] > widths[j + 1])
                    swap(&widths[j], &widths[j + 1]);
    }
    //worl 0 for lengths
    else if (worl == 1)
    {
        int i, j;
        for (i = 0; i < n - 1; i++)
            // Last i elements are already in place
            for (j = 0; j < n - i - 1; j++)
                if (lengths[j] > lengths[j + 1])
                    swap(&lengths[j], &lengths[j + 1]);
    }
}

//distance in mm
double distance = 195;
//sensor height in mm
double sensor_height_width = 3.2;
double sensor_height_length = 2.4;
//focal_length in mm
double focal_length = 3.21;
//total pixels in camera --> width
double image_width = 320;
//total pixels in camera --> height
double image_length = 240;
//maximum width in mm
double max_width = 149;
//maximum length in mm
double max_length = 103;
double volume = 0;
//boolean flag to indicate whether measurement started
bool started = false;
//counts the number of times measurement was done
int count = 0;
//boolean flag to check if the last measurement was a block
bool last_block = true;


//calulation is done filming 2 sides
//1st side is the base side (to get the area of the base)
//2nd side is the rear view of the object
//each side is filmed 2 times as inner (biggest rectangle that can be contained by the actual object) and outer boxes (smallest rectangle that can contain the object)
//the rectangles are the blocks visible in huskylens during measurement
//x1, y1 is the width and the length of the inner box of side 1
//x2, y2 is the width and the length of the outer box of side 1
//x3, y3 is the width and the length of the inner box of side 2
//x4, y4 is the width and the length of the outer box of side 2
double x1 = 0, x2 = 0, x3 = 0, x4 = 0, y1 = 0, y2 = 0, y3 = 0, y4 = 0;


void loop()
{
    delay(1000);
    bool saved = false;
    //huskylens connection is unstable
    if (!huskylens.request())
        Serial.println(F("Fail to request data from HUSKYLENS, recheck the connection!"));
    //nothing is learned by huskylens yet
    else if (!huskylens.isLearned())
    {
        //measurement is not started yet
        if (!started)
        {
            Serial.println(F("Nothing learned, press learn button on HUSKYLENS to learn one!"));
        }
        //measurement started
        else
        {
            count++;
            //measurement for inner frame of side 2
            //all sides are done measuring
            //print result on serial monitor
            if(count % 4 == 0 && count != 1){
                Serial.print("Count:");
                Serial.println(count);
                Serial.println("saved inner frame of side 2");
                double median_width = widths[widths.size()/2];
                double median_length = lengths[lengths.size()/2];
                x4 = median_width;
                y4 = median_length;
                Vector <double> temp;
                Vector <double> temp1;
                widths = temp;
                lengths = temp;
                started = false;
                //approximation equation for the base area
                double A = ((x1+x2)*(y1+y2))/4;
                //approximation equation for the height of the rear side
                double h = pow((((x3+x4)*(y3+y4))/(4*x3*y4)), 1.585) * y3;
                volume = A * h;
                Serial.println("volume calcuation complete");
                Serial.println(volume);
                count = 0;
                
            }
            //measurement for outer frame of side 2
            else if(count % 3 == 0 && count != 1){
                Serial.print("Count:");
                Serial.println(count);
                Serial.println("saved outer frame of side 2");
                double median_width = widths[widths.size()/2];
                double median_length = lengths[lengths.size()/2];
                x3 = median_width;
                y3 = median_length;
                Vector <double> temp;
                Vector <double> temp1;
                widths = temp;
                lengths = temp;
                started = false;
            }
            //measurement for inner frame of side 1
            else if (count % 2 == 0 && count != 1)
            {
                Serial.print("Count:");
                Serial.println(count);
                Serial.println("saved inner frame of side 1");
                double median_width = widths[widths.size()/2];
                double median_length = lengths[lengths.size()/2];
                x2 = median_width;
                y2 = median_length;
                Vector <double> temp;
                Vector <double> temp1;
                widths = temp;
                lengths = temp;
                started = false;
            }
            //measurement for inner frame of side 1
            else
            {
                Serial.print("Count:");
                Serial.println(count);
                Serial.println("saved outer frame of side 1");
                //median of the widths are taken, and the widths and lengths vector are reset
                //the start variable is set to false to wait for user measurement
                double median_width = widths[widths.size()/2];
                double median_length = lengths[lengths.size()/2];
                x1 = median_width;
                y1 = median_length;
                Vector <double> temp;
                Vector <double> temp1;
                widths = temp;
                lengths = temp;
                started = false;
                
            }
        }
    }
    //nothing is being captured in the screen of huskylens
    else if (!huskylens.available())
        Serial.println(F("No block or arrow appears on the screen!"));
    //huskylens is capturing a block
    else
    {
        started = true;
        Serial.println(F("###########"));
        while (huskylens.available())
        {
            HUSKYLENSResult result = huskylens.read();
            //calculations for blocks
            if (result.command == COMMAND_RETURN_BLOCK)
            {
                last_block = true;
                double object_width = result.width;
                double object_length = result.height;
                printResult(result);
                //the formula to calculate actual length
                double real_width = (distance * (object_width * sensor_height_width)) / (focal_length * image_width);
                //same formula to calculate actual width
                double real_length = (distance * (object_length * sensor_height_length)) / (focal_length * image_length);
                Serial.print("Real width in mm is: ");
                Serial.println(real_width);
                Serial.print("Real height in mm is: ");
                Serial.println(real_length);
                //push back to vector if an appropriate width and length was calculated
                //object cannot be bigger than the maximum value set
                if (real_width <= max_width && real_width > 0 && real_length <= max_length && real_length > 0)
                {
                    widths.push_back(real_width);
                    lengths.push_back(real_length);
                }
                //error message in case a wrong width or length was measured
                else{
                    if(real_width <= max_width && real_width > 0){
                       Serial.println("measured width of the object is bigger than limit or is smaller than 0");
                    }
                    else{
                       Serial.println("measured length of the object is bigger than limit or is smaller than 0");
                    }
                }
            }
            //calculations for lines
            //no algorithms for lines because it was too unstable and blocks are enough
            else
            {
                last_block = false;
            }
        }
        //print all the width or length that has been measured until now
        if (last_block)
        {
            bubbleSort(0, widths.size());
            bubbleSort(1, lengths.size());
            Serial.println("Printing widths: ");
            for (int i = 0; i < widths.size(); i++)
            {
                Serial.print(widths[i]);
                Serial.print(" ");
            }
            Serial.println();
            Serial.println("Printing lengths: ");
            for (int i = 0; i < lengths.size(); i++)
            {
                Serial.print(lengths[i]);
                Serial.print(" ");
            }
            Serial.println();
        }
        else
        {
        }
    }
}

void printResult(HUSKYLENSResult result)
{
    if (result.command == COMMAND_RETURN_BLOCK)
    {
        //Serial.println(String() + F("Block:xCenter=") + result.xCenter + F(",yCenter=") + result.yCenter + F(",width=") + result.width + F(",height=") + result.height + F(",ID=") + result.ID);
    }
    else if (result.command == COMMAND_RETURN_ARROW)
    {
        //Serial.println(String() + F("Arrow:xOrigin=") + result.xOrigin + F(",yOrigin=") + result.yOrigin + F(",xTarget=") + result.xTarget + F(",yTarget=") + result.yTarget + F(",ID=") + result.ID);
    }
    else
    {
        Serial.println("Object unknown!");
    }
}
