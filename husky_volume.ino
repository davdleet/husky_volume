#include "HUSKYLENS.h"
#include "SoftwareSerial.h"

HUSKYLENS huskylens;
SoftwareSerial mySerial(10, 11); // RX, TX
//HUSKYLENS green line >> Pin 10; blue line >> Pin 11
void printResult(HUSKYLENSResult result);

void setup() {
    Serial.begin(115200);
    mySerial.begin(9600);
    while (!huskylens.begin(mySerial))
    {
        Serial.println(F("Begin failed!"));
        Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protocol Type>>Serial 9600)"));
        Serial.println(F("2.Please recheck the connection."));
        delay(100);
    }
}

//Vector for dynamic container
template<typename Data>
class Vector {
   size_t d_size; // Stores no. of actually stored objects
   size_t d_capacity; // Stores allocated capacity
   Data *d_data; // Stores data
   public:
     Vector() : d_size(0), d_capacity(0), d_data(0) {}; // Default constructor
     Vector(Vector const &other) : d_size(other.d_size), d_capacity(other.d_capacity), d_data(0) { d_data = (Data *)malloc(d_capacity*sizeof(Data)); memcpy(d_data, other.d_data, d_size*sizeof(Data)); }; // Copy constuctor
     ~Vector() { free(d_data); }; // Destructor
     Vector &operator=(Vector const &other) { free(d_data); d_size = other.d_size; d_capacity = other.d_capacity; d_data = (Data *)malloc(d_capacity*sizeof(Data)); memcpy(d_data, other.d_data, d_size*sizeof(Data)); return *this; }; // Needed for memory management
     void push_back(Data const &x) { if (d_capacity == d_size) resize(); d_data[d_size++] = x; }; // Adds new value. If needed, allocates more space
     size_t size() const { return d_size; }; // Size getter
     Data const &operator[](size_t idx) const { return d_data[idx]; }; // Const getter
     Data &operator[](size_t idx) { return d_data[idx]; }; // Changeable getter
   private:
     void resize() { d_capacity = d_capacity ? d_capacity*2 : 1; Data *newdata = (Data *)malloc(d_capacity*sizeof(Data)); memcpy(newdata, d_data, d_size * sizeof(Data)); free(d_data); d_data = newdata; };// Allocates double the old space
};

//Vector to store widths
Vector <int> widths;
//Vector to store lengths
Vector <int> lengths;


//swap two ints
void swap(int *xp, int *yp)
{
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}
 
// A function to implement bubble sort
void bubbleSort(int worl, int n)
{
    if(worl == 0){
      int i, j; 
      for (i = 0; i < n-1; i++)    
       
      // Last i elements are already in place
      for (j = 0; j < n-i-1; j++)
          if (widths[j] > widths[j+1])
              swap(&widths[j], &widths[j+1]); 
    }
    else if(worl == 1){
      int i, j; 
      for (i = 0; i < n-1; i++)    
       
      // Last i elements are already in place
      for (j = 0; j < n-i-1; j++)
          if (lengths[j] > lengths[j+1])
              swap(&lengths[j], &lengths[j+1]); 
    }
}

//huskylens.writeAlgorithm(ALGORITHM_NAME) to change algorithms
//


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


double side1_area = 0;
//change to height of side 2
double side2_length = 0;
double side2_area = 0;
double volume = 0;
bool started = false;
int count = 0;
int measurement_num = 0;
bool last_block = true;

void loop() {
    delay(1000);
    bool saved = false;
    if (!huskylens.request()) Serial.println(F("Fail to request data from HUSKYLENS, recheck the connection!"));
    else if(!huskylens.isLearned()) 
    {
      if(!started){
        Serial.println(F("Nothing learned, press learn button on HUSKYLENS to learn one!"));
      }
      else{
        count++;
        if(count % 2 == 0 && count != 1){
          measurement_num++;
          Serial.println("Saved side 2");
          Serial.print("Measurement #");
          Serial.print(measurement_num);
          Serial.println(" complete. continue for next measurement");
          //int median_width = widths[widths.size()/2];
          int median_length = lengths[lengths.size()/2];
          //side2_area = median_width * median_length;
          side2_length = median_length;
          Serial.print("side1 area: ");
          Serial.println(side1_area);
          Serial.print("side2 length: ");
          Serial.println(side2_length);
          volume = side1_area * side2_length;
          Serial.print("Resulting volume in mm^3: ");
          Serial.println(volume);
          Vector <int> temp;
          Vector <int> temp1;
          widths = temp;
          lengths = temp1;
          started = false; 
        }
        else{
          Serial.println("Saved side 1");
          int median_width = widths[widths.size()/2];
          int median_length = lengths[lengths.size()/2];
          side1_area = median_width * median_length;
          Vector <int> temp;
          Vector <int> temp1;
          widths = temp;
          lengths = temp1;
          started = false;
        }
      }
    }
    else if(!huskylens.available()) Serial.println(F("No block or arrow appears on the screen!"));
    else
    {
        started = true;
        Serial.println(F("###########"));
        while (huskylens.available())
        {
            HUSKYLENSResult result = huskylens.read();
            //calculations for blocks
            if(result.command == COMMAND_RETURN_BLOCK){
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
              if(real_width <= max_width && real_width > 0 && real_length <= max_length && real_length > 0){
                 widths.push_back(real_width);
                 lengths.push_back(real_length);
              }      
            }
            //calculations for lines
            else{
              last_block = false;
            }
        
        }
        if(last_block){
          bubbleSort(0, widths.size());
          bubbleSort(1, lengths.size());
          Serial.println("Printing widths: ");
          for(int i = 0; i < widths.size(); i++){
             Serial.print(widths[i]);
             Serial.print(" ");
          }
          Serial.println();
          Serial.println("Printing lengths: ");
          for(int i = 0; i < lengths.size(); i++){
             Serial.print(lengths[i]);
             Serial.print(" ");
          }
          Serial.println();
          Serial.println(huskylens.customText("helo", 120, 120));
        }
        else{
          
        }
    }
    
}

void printResult(HUSKYLENSResult result){
    if (result.command == COMMAND_RETURN_BLOCK){
        Serial.println("Test");
        Serial.println(String()+F("Block:xCenter=")+result.xCenter+F(",yCenter=")+result.yCenter+F(",width=")+result.width+F(",height=")+result.height+F(",ID=")+result.ID);
    }
    else if (result.command == COMMAND_RETURN_ARROW){
        Serial.println(String()+F("Arrow:xOrigin=")+result.xOrigin+F(",yOrigin=")+result.yOrigin+F(",xTarget=")+result.xTarget+F(",yTarget=")+result.yTarget+F(",ID=")+result.ID);
    }
    else{
        Serial.println("Object unknown!");
    }
}
