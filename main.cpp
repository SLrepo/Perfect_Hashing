# include <iostream>
# include <vector>
# include <list>
#include <cstdlib>
#include "image.h"
#include <algorithm>
# include <cmath>

// ===================================================================================================
// ===================================================================================================

axis LargestOffset(Image<int>& counts, const int& offset_size);
//Offset SearchThrough(const std::list<axis>& list, const Image<Color>& input, Image<Color> &hash_data, int offset_size, int hash_data_size);
bool NoCommonFactor(int a, int b);
bool SearchThrough(const int x, const int y, const std::list<axis>& list, const Image<Color>& input, Image<Offset>& offset, Image<Color> &hash_data, int offset_size, int hash_data_size);

void Compress(const Image<Color> &input, 
              Image<bool> &occupancy, Image<Color> &hash_data, Image<Offset> &offset) {
  //set occupancy.
  occupancy.Allocate (input.Width(), input.Height());
  int nonWhite=0;
  unsigned int i;
  unsigned int j;
  for (i=0; i<input.Width(); i++){
    for(j=0; j<input.Height(); j++){
      if (input.GetPixel(i,j).isWhite())
        occupancy.SetPixel(i,j,false);
      else{
        occupancy.SetPixel(i,j,true);
        nonWhite++;
      }
    }
  }
  std::cout << "there are " << nonWhite << " non white pixels."<<std::endl;
  //
  int hash_data_size =sqrt(1.01* nonWhite)+1;
  int offset_size=sqrt(nonWhite/4)+1;

  bool ok = false; 
  //int hash_increase=0;
  //int offset_increase=0;
  int iteration=0;
  offset_size=offset_size-1;
  Image<std::list<axis> > content;
  Image<int> counts;
  std::vector<axis> order;
  axis large(0,0);
  std::list<axis> set_push_back;

while (!ok){
  iteration++;
  if (iteration%2==0){
    hash_data_size++;
    //offset_increase=0;
  }
  else{
    offset_size++;
    std::cout << "increase offset_size"<<std::endl;
    //hash_increase=0;
  }
  //hash_data_size=hash_data_size+hash_increase;
  //offset_size=offset_size+offset_increase;
  while (!NoCommonFactor(hash_data_size, offset_size)){
    offset_size++;
  }
  std::cout << "offset size is "<<offset_size<<std::endl;
  std::cout << "hash_data_size is "<< hash_data_size <<std::endl;

  offset.Allocate(offset_size, offset_size);
  offset.SetAllPixels(Offset(0,0));

  hash_data.Allocate(hash_data_size, hash_data_size);
  hash_data.SetAllPixels(Color(255,255,255));

  content.Allocate(offset_size, offset_size);
  counts.Allocate(offset_size, offset_size);
  counts.SetAllPixels(0);



  for (i=0; i<input.Width(); i++){
    for(j=0; j<input.Height(); j++){
      if (occupancy.GetPixel(i,j)==true){
        set_push_back=content.GetPixel(i%offset_size, j%offset_size);
        set_push_back.push_back(axis(i,j));
        content.SetPixel(i%offset_size, j%offset_size, set_push_back);
        counts.SetPixel(i%offset_size, j%offset_size, counts.GetPixel(i%offset_size, j%offset_size)+1);
        //std::cout<< "( "<< i <<" , "<<j<<" ) is given to offset pixel ( "<< i%offset_size << " , "<<j%offset_size<<" )"<<std::endl;
      }
    }
  }
  //std::cout << "the size of content is"<<content.GetPixel(1,0).size()<<std::endl;
  order.clear(); //stores the current axis of the offset that has the largest num of pixels.
  
  for (i=0;i<offset_size*offset_size;i++){
    large=LargestOffset(counts, offset_size);
    if (large.x!=-1 || large.y!=-1)
       order.push_back(large);
    //std::cout << "( "<<large.x <<" , "<<large.y<<" ) is pushed into order. "<<std::endl;
  }
  std::cout << "the size of order is "<< order.size()<<std::endl;
  
  for(i=0; i<order.size(); i++){
    ok=SearchThrough(order[i].x, order[i].y, content.GetPixel(order[i].x, order[i].y), input, offset, hash_data, offset_size, hash_data_size);
    //std::cout << "there is "<<ok <<" results for ("<< order[i].x << ","<<order[i].y<< ") offset."<<std::endl;
    if (ok==false) break;
  }
  std::cout << "the size of offset is "<< offset.Width()<< std::endl;
}

}

//find the offset cubic that has the largest num of color;
axis LargestOffset(Image<int>& counts, const int& offset_size){
  axis a(0,0);
  int largest=0;
  int i;
  int j;

  for (i=0; i<offset_size; i++){
    for (j=0; j<offset_size; j++){
      if(counts.GetPixel(i,j)>=largest){
        largest=counts.GetPixel(i,j);
        a.x=i;
        a.y=j;
      }
    }
  }

  if (counts.GetPixel(a.x,a.y)==0)
    return axis(-1,-1);

  counts.SetPixel(a.x,a.y,0);  
  return a;
}

bool SearchThrough(const int x, const int y, const std::list<axis>& list, const Image<Color>& input, Image<Offset>& offset, Image<Color> &hash_data, int offset_size, int hash_data_size ) {
  bool taken=true;
  Offset of(0, 0);
  int test=1;
  int i;
  int j;
  std::list<axis>::const_iterator iter;
  //std::cout << "start to test the pixels in ("<< x << " , " << y << ") offset. "<< std::endl;
  //std::cout << offset_size << std::endl;
  //std::cout << "list size is "<<list.size() << std::endl;
  for (i=0; i<=15; i++){
    for (j=0; j<=15; j++){
      //taken=false;
      test=1;
      //std::cout << i << " , " << j << std::endl;
      for (iter=list.begin(); iter!=list.end(); iter++){
        if (!hash_data.GetPixel( ((iter->x)+i)%hash_data_size , ((iter->y)+j)%hash_data_size).isWhite()){
           test=test*0;
           //std::cout << (iter->x+i)%hash_data_size << " , " << (iter->y+j)%hash_data_size << "is taken"<<std::endl;
           //iter=list.end();
        }
      }
      if (test==1) taken=false;
      //std::cout << "taken is  "<< taken << ". "<< i << " , "<< j <<std::endl;
      if(taken==false){
        //std::cout << "found offset value (" << i <<" , "<< j <<") haha"<<std::endl;
        of.dx=i;
        of.dy=j;
        for (std::list<axis>::const_iterator iter1=list.begin(); iter1!=list.end(); iter1++){
          hash_data.SetPixel((iter1->x+i)%hash_data_size, (iter1->y+j)%hash_data_size, input.GetPixel(iter1->x, iter1->y));
          //std::cout << (iter1->x+i)%hash_data_size << " , "<< (iter1->y+j)%hash_data_size << " set as R: " 
          //<< input.GetPixel(iter1->x, iter1->y).r<<". G: "<<input.GetPixel(iter1->x, iter1->y).g<<". B: "<<input.GetPixel(iter1->x, iter1->y).b << std::endl;
        }
        offset.SetPixel(x, y, of);
        //std::cout << "found offset value (" << of.dx <<" , "<< of.dy <<") for offset pixel ( "<<x <<" , "<<y<<" )."<<std::endl;
        return true;
      }
    }
  }
  return false;
}

bool NoCommonFactor(int a, int b){
    int t;
    if (a>b)
      t=b;
    else
      t=a;
    for (int i =2; i<= t; i++){
      if (a%i==0 && b%i==0)
        return false;
    }
    return true;
}

void UnCompress(const Image<bool> &occupancy, const Image<Color> &hash_data, const Image<Offset> &offset, 
                Image<Color> &output) {
  output.Allocate(occupancy.Width(), occupancy.Height());
  for(int i=0; i<occupancy.Width(); i++){
    for(int j=0; j<occupancy.Height(); j++){
      if (occupancy.GetPixel(i, j)){
        output.SetPixel(i, j, hash_data.GetPixel( (i+offset.GetPixel(i%offset.Width(), j%offset.Width()).dx )%hash_data.Width(), 
        (j+offset.GetPixel(i%offset.Width(), j%offset.Width()).dy )%hash_data.Width() ));
      }
    }
  }

}


// ===================================================================================================
// ===================================================================================================

// Takes in two 24-bit color images as input and creates a b&w output
// image (black where images are the same, white where different)
void Compare(const Image<Color> &input1, const Image<Color> &input2, Image<bool> &output) {

  // confirm that the files are the same size
  if (input1.Width() != input2.Width() ||
      input1.Height() != input2.Height()) {
    std::cerr << "Error: can't compare images of different dimensions: " 
         << input1.Width() << "x" << input1.Height() << " vs " 
         << input2.Width() << "x" << input2.Height() << std::endl;
  } else {
    // make sure that the output images is the right size to store the
    // pixel by pixel differences
    output.Allocate(input1.Width(),input1.Height());
    int count = 0;
    for (int i = 0; i < input1.Width(); i++) {
      for (int j = 0; j < input1.Height(); j++) {
        Color c1 = input1.GetPixel(i,j);
        Color c2 = input2.GetPixel(i,j);
        if (c1.r == c2.r && c1.g == c2.g && c1.b == c2.b)
          output.SetPixel(i,j,true);
        else {
          count++;
          output.SetPixel(i,j,false);
        }
      }      
    }     

    // confirm that the files are the same size
    if (count == 0) {
      std::cout << "The images are identical." << std::endl;
    } else {
      std::cout << "The images differ at " << count << " pixel(s)." << std::endl;
    }
  }
}

// ===================================================================================================
// ===================================================================================================

// to allow visualization of the custom offset image format
void ConvertOffsetToColor(const Image<Offset> &input, Image<Color> &output) {
  // prepare the output image to be the same size as the input image
  output.Allocate(input.Width(),input.Height());
  for (int i = 0; i < output.Width(); i++) {
    for (int j = 0; j < output.Height(); j++) {
      // grab the offset value for this pixel in the image
      Offset off = input.GetPixel(i,j);
      // set the pixel in the output image
      int dx = off.dx;
      int dy = off.dy;
      assert (dx >= 0 && dx <= 15);
      assert (dy >= 0 && dy <= 15);
      // to make a pretty image with purple, cyan, blue, & white pixels:
      int red = dx * 16;
      int green = dy *= 16;
      int blue = 255;
      assert (red >= 0 && red <= 255);
      assert (green >= 0 && green <= 255);
      output.SetPixel(i,j,Color(red,green,blue));
    }
  }
}

// ===================================================================================================
// ===================================================================================================

void usage(char* executable) {
  std::cerr << "Usage:  4 options" << std::endl;
  std::cerr << "  1)  " << executable << " compress input.ppm occupancy.pbm data.ppm offset.offset" << std::endl;
  std::cerr << "  2)  " << executable << " uncompress occupancy.pbm data.ppm offset.offset output.ppm" << std::endl;
  std::cerr << "  3)  " << executable << " compare input1.ppm input2.ppm output.pbm" << std::endl;
  std::cerr << "  4)  " << executable << " visualize_offset input.offset output.ppm" << std::endl;
}

// ===================================================================================================
// ===================================================================================================

int main(int argc, char* argv[]) {
  if (argc < 2) { usage(argv[0]); exit(1); }

  if (argv[1] == std::string("compress")) {
    if (argc != 6) { usage(argv[0]); exit(1); }
    // the original image:
    Image<Color> input;
    // 3 files form the compressed representation:
    Image<bool> occupancy;
    Image<Color> hash_data;
    Image<Offset> offset;
    input.Load(argv[2]);
    Compress(input,occupancy,hash_data,offset);
    // save the compressed representation
    occupancy.Save(argv[3]);
    hash_data.Save(argv[4]);
    std::cout << "here1"<<std::endl;
    offset.Save(argv[5]);

  } else if (argv[1] == std::string("uncompress")) {
    if (argc != 6) { usage(argv[0]); exit(1); }
    // the compressed representation:
    Image<bool> occupancy;
    Image<Color> hash_data;
    Image<Offset> offset;
    occupancy.Load(argv[2]);
    hash_data.Load(argv[3]);
    offset.Load(argv[4]);
    // the reconstructed image
    Image<Color> output;
    UnCompress(occupancy,hash_data,offset,output);
    // save the reconstruction
    output.Save(argv[5]);
  
  } else if (argv[1] == std::string("compare")) {
    if (argc != 5) { usage(argv[0]); exit(1); }
    // the original images
    Image<Color> input1;
    Image<Color> input2;    
    input1.Load(argv[2]);
    input2.Load(argv[3]);
    // the difference image
    Image<bool> output;
    Compare(input1,input2,output);
    // save the difference
    output.Save(argv[4]);

  } else if (argv[1] == std::string("visualize_offset")) {
    if (argc != 4) { usage(argv[0]); exit(1); }
    // the 8-bit offset image (custom format)
    Image<Offset> input;
    input.Load(argv[2]);
    // a 24-bit color version of the image
    Image<Color> output;
    ConvertOffsetToColor(input,output);
    output.Save(argv[3]);

  } else {
    usage(argv[0]);
    exit(1);
  }
}

// ===================================================================================================
// ===================================================================================================
