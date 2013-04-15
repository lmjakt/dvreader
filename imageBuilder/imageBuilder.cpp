#include "imageBuilder.h"
#include "MaskMaker.h"
#include "panels/fileSet.h"
#include "opengl/glImage.h"
#include "centerFinder.h"
#include "imStack.h"
#include "dir_k_cluster.h"
#include "blob_mt_mapper.h"
#include "CellOutlines.h"
#include "CellCollection.h"
#include "CellParCollector.h"
#include "Drawer.h"
#include "PointDilater.h"
#include "../dataStructs.h"
#include "../datastructs/channelOffset.h"
#include "../image/two_d_background.h"
#include "../image/spectralResponse.h"
#include "../tiff/tiffReader.h"
#include "../distchooser/distChooser.h"
#include "../distchooser/tabWidget.h"
#include "../stat/stat.h"
#include "../image/gaussian.h"
#include "../image/blobModel.h"
#include "../image/imageAnalyser.h"
#include "../panels/stack_stats.h"
#include "../panels/borderInformation.h"
#include "../linGraph/linePlotter.h"
#include "BlobSetPlotter.h"
#include "../linGraph/distPlotter.h"    

#include "../scatterPlot/scatterPlotter.h"
#include "../spotFinder/perimeterWindow/perimeterWindow.h"
//#include "../spotFinder/spotMapper/nearestNeighborMapper.h"
#include "../spotFinder/spotMapper/NNMapper2.h"  

#include "../open_cl/MIPf_cl.h"
#include "../open_cl/ImExpand_cl.h"

#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <unistd.h>
#include <QTextStream>
#include <QSemaphore>

#include <QFile>
#include <QKeyEvent>

using namespace std;

ImageBuilder::ImageBuilder(FileSet* fs, vector<channel_info>& ch, QObject* parent)
  : QObject(parent)
{
  data = fs;
  imageAnalyser = 0;  // make a new one if necessary.. 
  texture_size = 1024;  // this must be an even 2^n
  maskMaker = 0;
  int texColNo = (data->pwidth() % texture_size) ? 1 + (data->pwidth() / texture_size) : data->pwidth() / texture_size;
  int texRowNo = (data->pheight() % texture_size) ? 1 + (data->pheight() / texture_size) : data->pheight() / texture_size;

  image = new GLImage(texColNo, texRowNo, texture_size);
  image->resize(texture_size, texture_size);
  image->setCaption("ImageBuilder Image");

  rgbData = new float[ data->pwidth() * data->pheight() * 3];
  overlayData = new unsigned char[4 * data->pwidth() * data->pheight() ];
  textOverlayData = new unsigned char[4 * data->pwidth() * data->pheight() ];
  memset((void*)overlayData, 0, sizeof(unsigned char) * 4 * data->pwidth() * data->pheight());
  memset((void*)textOverlayData, 0, sizeof(unsigned char) * 4 * data->pwidth() * data->pheight());
  sBuffer = new unsigned short[ data->pwidth() * data->pheight() ];
  channels = ch;//class ImageAnalyser;

  blobSetPlotter = 0;

  distTabs = new TabWidget();
  distTabs->setCaption("ImageBuilder Distributions");
  for(uint i=0; i < channels.size(); ++i){
    channels[i].finfo = data->channelInfo(i);
    ostringstream ss;
    ss << data->channel(i);
    distributions.push_back(new DistChooser(ss.str(), 100));
    distTabs->addTab(distributions.back(), ss.str().c_str());
  }
  
  // set up map  of various pipe slice functions.
  pipe_slice_functions["slice"] = &ImageBuilder::p_slice;
  pipe_slice_functions["g_blur"] = &ImageBuilder::p_gaussian;
  pipe_slice_functions["lg_blur"] = &ImageBuilder::pl_gaussian;
  pipe_slice_functions["bg_sub"] = &ImageBuilder::p_sub_bg;
  pipe_slice_functions["ch_sub"] = &ImageBuilder::p_sub_channel;
  pipe_slice_functions["p_mean"] = &ImageBuilder::p_mean_panel;

  // a set of general functions taking instances of the f_parameter class
  // as an argument.
  general_functions["set_par"] = &ImageBuilder::set_par;
  general_functions["read_commands"] = &ImageBuilder::read_commands_from_file;
  general_functions["set_panel_bias"] = &ImageBuilder::setPanelBias;
  general_functions["set_frame_bgpar"] = &ImageBuilder::setFrameBackgroundPars;
  general_functions["find_center"] = &ImageBuilder::findCenter;
  general_functions["make_stack"] = &ImageBuilder::makeImStack;
  general_functions["cluster_voxels"] = &ImageBuilder::clusterVoxels;
  general_functions["f_project"] = &ImageBuilder::build_fprojection;
  general_functions["fg_project"] = &ImageBuilder::build_fgprojection;
  general_functions["add_slice"] = &ImageBuilder::addSlice;
  general_functions["blur_stack"] = &ImageBuilder::blurStack;
  general_functions["z_mult_stack"] = &ImageBuilder::stack_mult_z_nbor;
  general_functions["deblur_stack"] = &ImageBuilder::deBlurStack;
  general_functions["sub_stack"] = &ImageBuilder::subStack;
  general_functions["stack_bgsub"] = &ImageBuilder::subStackBackground;
  general_functions["loop_stack"] = &ImageBuilder::loopStack;

  general_functions["loop_xz"] = &ImageBuilder::loopXZ;
  general_functions["stack_xz"] = &ImageBuilder::stack_xzSlice;
  general_functions["stack_yz"] = &ImageBuilder::stack_yzSlice;
  general_functions["stack_project"] = &ImageBuilder::stack_project;

  general_functions["stack_project_cl"] = &ImageBuilder::stack_project_cl;
  general_functions["stack_expand_cl"] = &ImageBuilder::stack_expand_cl;

  general_functions["set_stack_par"] = &ImageBuilder::setStackPar;
  general_functions["map_blobs"] = &ImageBuilder::stack_map_blobs;
  general_functions["train_models"] = &ImageBuilder::trainBlobSetModels;
  general_functions["map_perimeters"] = &ImageBuilder::map_perimeters;
  general_functions["draw_perimeters"] = &ImageBuilder::project_perimeters;
  general_functions["map_blobs_to_perimeters"] = &ImageBuilder::map_blobs_to_perimeters;
  general_functions["compare_model"] = &ImageBuilder::compare_model;
  general_functions["blob_pars"] = &ImageBuilder::plot_blob_pars;
  general_functions["blob_set_dist"] = &ImageBuilder::plot_blob_set_dist;
  general_functions["plot_blob_sets"] = &ImageBuilder::plot_blob_sets;
  general_functions["plot_cell_blob_pars"] = &ImageBuilder::plot_cell_blob_pars;
  general_functions["merge_blobs"] = &ImageBuilder::mergeBlobs;
  general_functions["read_criteria"] = &ImageBuilder::readBlobCriteria;
  general_functions["project_blobs"] = &ImageBuilder::project_blob_collections;
  general_functions["project_blob_sets"] = &ImageBuilder::project_blob_sets;
  general_functions["dilate_blobs"] = &ImageBuilder::dilate_blob_sets;
  general_functions["project_blob_ids"] = &ImageBuilder::project_blob_ids;
  general_functions["make_cell_mask"] = &ImageBuilder::make_cell_mask;
  general_functions["edit_cell"] = &ImageBuilder::modifyCellPerimeter;
  general_functions["make_cells"] = &ImageBuilder::make_cells;
  general_functions["draw_cells"] = &ImageBuilder::draw_cells;
  general_functions["draw_cell"] = &ImageBuilder::draw_cell;
  general_functions["edit_cells"] = &ImageBuilder::modifyCells;
  general_functions["reassign_blobs"] = &ImageBuilder::reassign_blobs_cells;
  general_functions["set_cell_blobs"] = &ImageBuilder::set_cell_blobs;
  general_functions["set_nuclear_signals"] = &ImageBuilder::set_nuclear_signals;
  general_functions["export_cells"] = &ImageBuilder::export_cell_summary;
  general_functions["write_cells"] = &ImageBuilder::write_cells_borders;
  general_functions["read_cells"] = &ImageBuilder::read_cells_from_file;
  general_functions["draw_model"] = &ImageBuilder::draw_blob_model;
  general_functions["list"] = &ImageBuilder::list_objects;
  general_functions["report_parameter"] = &ImageBuilder::report_parameter;
  general_functions["set_bleach_counts"] = &ImageBuilder::set_bleach_counts;
  general_functions["bleach_count_p"] = &ImageBuilder::bleach_count_p;
  general_functions["bleach_count_map"] = &ImageBuilder::bleach_count_map;
  general_functions["plot_blob_bleach_count"] = &ImageBuilder::plot_blob_bleach_count;
  general_functions["plot_bleaching"] = &ImageBuilder::plot_bleaching;

  general_functions["setCenter"] = &ImageBuilder::setImageCenter;
  //general_functions["p_mean"] = &ImageBuilder::p_mean_panel;  // may be useful to do things like blurring.

}

ImageBuilder::~ImageBuilder()
{
  delete image;
  delete []rgbData;
  delete []sBuffer;
  delete []overlayData;
  delete []textOverlayData;
  // I may need to delete the distributions separately, not sure,,
  delete distTabs;
  for(map<QString, ImStack*>::iterator it=imageStacks.begin();
      it != imageStacks.end(); ++it)
    delete (*it).second;
  if(imageAnalyser)
    delete imageAnalyser;
  delete maskMaker;
}

bool ImageBuilder::buildSlice(std::set<unsigned int> wi, unsigned int slice)
{
  memset((void*)rgbData, 0, sizeof(float) * data->pwidth() * data->pheight() * 3);
  for(uint i=0; i < channels.size(); ++i)
    channels[i].include = (bool)wi.count(i);
  if( !data->readToRGB(rgbData, 0, 0, data->pwidth(), data->pheight(), slice, channels) )
    return(false);
  
  setRGBImage(rgbData, data->pwidth(), data->pheight());
  return(true);
}

bool ImageBuilder::gaussianSlice(set<unsigned int> wi, unsigned int slice, unsigned int g_radius)
{
  vector<unsigned int> ch;
  for(set<unsigned int>::iterator it=wi.begin(); it != wi.end(); ++it){
    if((*it) < channels.size())
      ch.push_back(*it);
  }
  if(!ch.size()){
    cerr << "ImageBuilder::gaussianSlice no good channels selected" << endl;
    return(false);
  }
  float** images = getFloatImages(ch, slice);
  float** bl_images = new float*[ch.size()];
  for(uint i=0; i < ch.size(); ++i){
    bl_images[i] = gaussian_blur_2d(images[i], data->pwidth(), data->pheight(), g_radius);
  }
  memset((void*)rgbData, 0, sizeof(float) * 3 * data->pwidth() * data->pheight());
  for(uint i=0; i < ch.size(); ++i)
    toRGB(bl_images[i], rgbData, channels[ch[i]], data->pwidth() * data->pheight());
  deleteFloats(images, ch.size());
  deleteFloats(bl_images, ch.size());
  setRGBImage(rgbData, data->pwidth(), data->pheight());
  return(true);
}

// At the moment, this uses a blurred image to estimate the background.
// But it still is not behaving as well as I was hoping.
bool ImageBuilder::buildSpectrallySubtractedImage(std::set<unsigned int> wi, unsigned int slice)
{
  if(bg_spectrum.size() < 2){
    cerr << "Unable to build spectrally subtracted image. bg_spectrum too small: " << bg_spectrum.size() << endl;
    return(false);
  }
  for(map<unsigned int, float>::iterator it=bg_spectrum.begin();
      it != bg_spectrum.end(); it++){
    if(!wi.count(it->first)){
      cerr << "Unable to build spectrally subtracted image. Not enough channels specified : " << it->first << endl;
      return(false);
    }
  }
  // and select the known things.
  vector<unsigned int> ch;
  for(set<unsigned int>::iterator it=wi.begin(); it != wi.end(); ++it){
    if((*it) < channels.size())
      ch.push_back(*it);
  }
  
  // get all the images..
  float** images = new float*[ch.size()];
  // TEMPORARY HACK TRY TO BLUR IMAGES BEFORE CALCULATING THE BACKGROUND
  float** bl_images = new float*[ch.size()];
  // keep the background spectrum in the bgr array (background responses)
  float* bgr = new float[ch.size()];
  for(uint i=0; i < ch.size(); ++i){
    images[i] = new float[ data->pwidth() * data->pheight() ];
    memset((void*)images[i], 0, sizeof(float) * data->pwidth() * data->pheight());
    data->readToFloat(images[i], 0, 0, slice, data->pwidth(), data->pheight(), 1, ch[i]);
    bl_images[i] = gaussian_blur_2d(images[i], data->pwidth(), data->pheight(), 3);
    if(bg_spectrum.count(ch[i])){
      bgr[i] = bg_spectrum[ch[i]];
    }else{
      bgr[i] = -1;
    }
  }
  // and then go through all of the pixel positions, estimate a max background value, and
  // subtract from the channel values.
  unsigned int l = data->pwidth() * data->pheight();
  float bg = 0;
  float tbg = 0;
  // The speed of the below can be improved by making sure that we only
  // go through the channels for which the background rate has been estimated.
  // To do that however, we would need another layer of index translation, which
  // makes it easier to introduce stupid bugs. So I'll leave this slow at the moment.
  for(unsigned int i=0; i < l; ++i){
    bg = -1;
    for(unsigned int j=0; j < ch.size(); ++j){
      if(bgr[j] <= 0)
	continue;
      if(bg < 0){         // has not been estimated for this pixel yet
	bg = bl_images[j][i] / bgr[j];
	continue;
      }
      tbg = bl_images[j][i] / bgr[j];
      bg = tbg < bg ? tbg : bg;
    }
    // And then subtract. This will leave one channel with a 0 value.
    // That's a little bit too harsh, and it means that we need to have
    // one empty background channel; can't have full labelling (3 colours).
    // which is a bit of a pity.
    if(bg <= 0)
      continue;
    for(unsigned int j=0; j < ch.size(); ++j){
      if(bgr[j] < 0)
	continue;
      //bl_images[j][i] /= (bg * bgr[j]);
      images[j][i] -= (bg * bgr[j]);
      //images[j][i] *= bl_images[j][i];
    }
  }
  // And then we simply have to convert this to an image of sorts..
  memset((void*)rgbData, 0, sizeof(float) * 3 * data->pwidth() * data->pheight());
  for(uint i=0; i < ch.size(); ++i){
    toRGB(images[i], rgbData, channels[ch[i]], data->pwidth() * data->pheight());
    delete []images[i];
  }
  delete []images;
  delete []bgr;
  deleteFloats(bl_images, ch.size());
  setRGBImage(rgbData, data->pwidth(), data->pheight());
  return(true);
}

bool ImageBuilder::buildProjection(std::vector<unsigned int> wi, unsigned int beg, unsigned int end)
{
  if(beg > end){
    unsigned int b = beg;
    beg = end;
    end = b;
  }
  // We need two short buffers:
  bool ok = true;
  memset((void*)rgbData, 0, sizeof(float) * data->plength() * 3);
  for(unsigned int i=0; i < wi.size(); ++i){
    if(wi[i] >= channels.size()){
      cerr << "ImageBuilder::buildProjection waveIndex too large : " << wi[i] << endl;
      ok = false;
      continue;
    }
    memset((void*)sBuffer, 0, sizeof(short) * data->pwidth() * data->pheight() );
    if(buildShortProjection(sBuffer, wi[i], beg, end))
      toRGB(sBuffer, rgbData, channels[wi[i]], data->pwidth() * data->pheight());
  }
  if(ok)
    setRGBImage(rgbData, data->pwidth(), data->pheight());
  return(ok);   // not very meaningful
}

void ImageBuilder::set_par(f_parameter& par)
{
  unsigned int wi;
  if(par.defined("sandb")){
    float s, b;
    if(par.param("s", s) &&
       par.param("b", b) &&
       par.param("wi", wi) )
      setScaleAndBias(wi, b, s);
  }
}

void ImageBuilder::read_commands_from_file(f_parameter& par)
{
  QString fileName;
  QString error;
  QTextStream qts(&error);
  if(!par.param("file", fileName)){
    qts << "Specify file containing the commands\n";
    warn(error);
    return;
  }
  QFile file(fileName);
  if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
    qts << "Unable to open file : " << fileName << "\n";
    warn(error);
    return;
  }
  QTextStream in(&file);
  vector<f_parameter> pars;
  vector<QString> commandLines;
  while(!in.atEnd()){
    QString line = in.readLine();
    if(line.length() && line.contains(QRegExp("\\w+"))){
      pars.push_back( f_parameter( line ) );
      commandLines.push_back(line);
    }
  }
  cout << "ImageBuilder::read_commands_from_file : read in a total of : " << pars.size() << " commands" << endl;
  
  for(unsigned int i=0; i < pars.size(); ++i){
    warn(commandLines[i]);
    general_command(pars[i]);
  }
  
}


void ImageBuilder::build_fprojection(f_parameter& par)
{
  int x = 0;
  int y = 0;
  int width = data->pwidth();
  int height = data->pheight();
  int beg = 0;
  int end = data->sectionNo() - 1;
  vector<int> waves;
  bool use_cmap=true;
  bool use_visible=false;
  // override the defaults if specified.
  par.param("beg", beg);
  par.param("end", end);

  if(!par.param("wi", ',', waves)){
    cerr << "build_fprojection you must specify the wave indexes wi=1,2,3 or something" << endl;
    return;
  }
  par.param("w", width); par.param("h", height); par.param("x", x); par.param("y", y);
  par.param("cmap", use_cmap);

  // if use_visible is true update only for the visible part of the image.
  par.param("use_visible", use_visible);
  if(use_visible){
    if(!setVisible(x, y, width, height))
      return;
  }
  QString stackName;
  bool saveStack = par.param("f_stack", stackName);
  if(saveStack)
    warn("build_fprojection no stack saving mechanism set yet. Use fg_project with r=0 instead\n");
  bool clear = true;
  par.param("clear", clear);
  bool clearAll = false;
  par.param("clear_all", clearAll);
  if(clearAll)
    memset((void*)rgbData, 0, sizeof(float) * data->plength() * 3);
  if(!clearAll && clear){
    for(int py=y; py < (y + height); ++py){
      memset((void*)(rgbData + 3 * (py * data->pwidth() + x)), 0, 3 * sizeof(float) * width);
    }
  }
  if(width <= 0 || height <= 0)
    return;
  float* pbuffer = new float[width * height];
  for(unsigned int i=0; i < waves.size(); ++i){
    if((unsigned int)waves[i] >= channels.size())
      continue;
    memset((void*)pbuffer, 0, sizeof(float) * width * height);
    if(buildFloatProjection(pbuffer, waves[i], x, y, width, height, (unsigned int)beg, (unsigned int)end, use_cmap)){
      pbuffer = modifyImage(x, y, width, height, pbuffer, par);
      toRGB(pbuffer, rgbData, channels[waves[i]], width * height, x, y, width, height);
    }
  }
  setRGBImage(rgbData, data->pwidth(), data->pheight());  // hmm 
  delete []pbuffer;
}

void ImageBuilder::build_fgprojection(f_parameter& par)
{
  unsigned int wi;
  if(!par.param("wi", wi)){
    cerr << "ImageBuilder::build_fgprojection need to specify wave index (wi)" << endl;
    return;
  }
  if(wi > channels.size()){
    cerr << "ImageBuilder::build_fgprojection wave index too large" << endl;
    return;
  }
  unsigned int radius = 0;
  par.param("r", radius);
  par.param("radius", radius);

  bool use_cmap = true;
  par.param("cmap", use_cmap);

  bool clear = true;  // do we clear the rgbImage ? 
  par.param("clear", clear); 

  int x=0; 
  int y=0;
  uint z=0;
  int w = data->pwidth();
  int h = data->pheight();
  uint d = data->sectionNo();
  
  ///// but using these defaults is a bit dangerous as this function will require 
  ///// a 3 * w * h * d * sizeof(float) memory. (The 3d gaussian takes two, and
  ///// one for the original stack..
  //    and then a few different ones for the various bits and pieces..
  
  par.param("x", x); par.param("y", y); par.param("z", z);
  par.param("w", w); par.param("h", h); par.param("d", d);
  
  if(w < 0 || h < 0 || x < 0 || y < 0){
    warn("Don't specify negative coordinates\n");
    return;
  }

  // sanity check.
  x = (int)x >= data->pwidth() ? 0 : x;
  y = (int)y >= data->pheight() ? 0 : y;
  z = (int)z >= data->sectionNo() ? 0 : z;
  w = (int)(x + w) < data->pwidth() ? w : (data->pwidth() - x);
  h = (int)(y + h) < data->pheight() ? h : (data->pheight() - y);
  d = (int)(z + d) < data->sectionNo() ? d : (data->sectionNo() - z);
  if(!w || !h || !d){
    cerr << "ImageBuilder::build_fgprojection bad dimensions specified" << endl;
    return;
  }
  // the user can also specify the use_visible flag
  bool use_visible = false;
  par.param("use_visible", use_visible);
  if(use_visible){
    if(!setVisible(x, y, w, h)){
      warn("setVisible returned false inappropriate regions specified\n");
      return;
    }
  }

  // which should be safe though we might run out of memory.. 
  float* stack = new float[w * h * d];
  memset((void*)stack, 0, sizeof(float) * w * h * d);
  // and then simply ..
  if(!data->readToFloat(stack, x, y, z, w, h, d, wi, use_cmap)){
    delete []stack;
    cerr << "ImageBuilder::fg_projection unable to read from panels. " << endl;
    return;
  }

  //  unsigned int blur_repeat = 1;
  //  par.param("brep", blur_repeat);
  float* bl_stack = stack;
  if(radius){
    //for(uint i=0; i < blur_repeat; ++i){
    bl_stack = gaussian_blur_3d(stack, w, h, d, radius);
    delete []stack;
    stack = bl_stack;
    //}
    if(!bl_stack){
      cerr << "ImageBuilder::fg_projection failed to blur stack" << endl;
      delete []stack;
      return;
    }
  }
  // and then simply make a projection from the stack.. 
  float* projection = projectStack_f(bl_stack, w, h, d);

  toRGB(projection, rgbData, channels[wi], (w * h), x, y, w, h, clear);
  delete []projection;
  QString stackName;
  if(par.param("fg_stack", stackName)){
    ImStack* stack = new ImStack(bl_stack, channels[wi], x, y, z, w, h, d);
    updateStacks(stackName, stack, true);
  }else{
    delete []bl_stack;
  }
  setRGBImage(rgbData, data->pwidth(), data->pheight());
}

void ImageBuilder::makeImStack(f_parameter& par)
{
  QString error;
  QTextStream qts(&error);
  // Need to specify:
  // 1. wave_indices (wi). Comma separated indices
  // 2. x, y, z positions
  // 3. width, height, depth
  // 4. a boolean indicating whether to use cmap (use_cmap) or not.. 
  // 5. a name of the image stack.
  //    which will be stored in the imageStacks map
  // then simply use the FileSet::imageStack(...) function.
  
  QString stackName;
  int x, y, z;
  x = y = z = 0;
  unsigned int w, h, d; // these all have to be specified no defaults
  w = (uint)data->pwidth();
  h = (uint)data->pheight();
  d = (uint)data->sectionNo();  // reasonable defaults.
  std::vector<unsigned int> wi;
  if(!par.param("stack", stackName) || !stackName.length())
    qts << "Please specify a stack name for the new stack\n";
  // if(!par.param("x", x) || !par.param("y", y) || !par.param("z", z))
  //   qts << "Please specify position of stack x=.. y=.. z=..\n";
  // if(!par.param("w", w) || !par.param("h", h) || !par.param("d", d))
  //   qts << "Please specify dimensions of stack w=.. h=.. d=..\n";
  par.param("x", x);
  par.param("y", y);
  par.param("z", z);
  par.param("w", w);
  par.param("h", h);
  par.param("d", d);   // optional, but not sanity checked. // should be done somewhere
  if(!par.param("wi", ',', wi))
    qts << "Please specify the wave indices to use\n";
  if(error.length()){
    warn(error);
    return;
  }
  bool use_cmap = true;
  par.param("cmap", use_cmap);
  ImStack* stack = data->imageStack(wi, x, y, z, w, h, d, use_cmap);
  
  if(!stack){
    warn("Error: data->imageStack() returned NULL stack");
    return;
  }
  // stack is supposed to get some channel info from data, but this isn't necessarily the
  // same as the current ones used here, so
  for(uint i=0; i < wi.size(); ++i){
    if(wi[i] < channels.size())
      stack->setChannelInfo(channels[wi[i]], i);
  }

  string text_file;
  if(par.param("export_text", text_file))
    stack->exportAscii(text_file);
  if(imageStacks.count(stackName))
    delete imageStacks[stackName];
  imageStacks[stackName] = stack;
  qts << "Successfully created image stack (" << stackName << ") with " << stack->ch() << " channels";
  warn(error);  // though not an error at this point.
  displayStack(stack); // displays the whole stack.
}

void ImageBuilder::clusterVoxels(f_parameter& par)
{
  QString error;
  QTextStream qts(&error);
  // Specify
  // 1. an Image stack from which to cluster voxels based on their spectral curves
  // 2. the number of clusters to obtain..
  // 3. the number of iterations to run.
  // 4. Optionally the name of the cluster object
  QString stackName;
  unsigned int k, n;
  if(!par.param("stack", stackName) || !imageStacks.count(stackName))
    qts << "Please specify an existing image stack : stack=..\n" ;
  if(!par.param("k", k) || !k)
    qts << "Specify the number of clusters as a non-negative number k=..\n";
  if(!par.param("n", n) || !n)
    qts << "Specify the number of iterations to run the cluster process for n=..\n";
  if(error.length()){
    warn(error);
    return;
  }
  // the image stack needs to have more than one channel..
  if(imageStacks[stackName]->ch() < 2){
    warn("The specified image stack has 0 or 1 channels, cannot cluster by direction");
    return;
  }
  QString clusterName = stackName;
  par.param("cluster", clusterName);
  if(voxel_clusters.count(clusterName))
    delete voxel_clusters[ clusterName ];

  Dir_K_Cluster* clusterObject = new Dir_K_Cluster( imageStacks[ stackName ] );
  voxel_clusters[ clusterName ] = clusterObject;
  // need some way of deleting..
  clusterObject->cluster(k, n);
  std::vector<std::vector<float> > centers = clusterObject->clusterCenters();
  for(unsigned int i=0; i < centers.size(); ++i){
    cout << i;
    for(unsigned int j=0; j < centers[i].size(); ++j)
      cout << "\t" << centers[i][j];
    cout << endl;
  }
  // Export the normalised stack..
  string ascii_file;
  if(par.param("export_text", ascii_file)){
    ImStack* data = clusterObject->vector_data();
    data->exportAscii(ascii_file);
  }

}

bool ImageBuilder::addBackground(unsigned int wi, unsigned int slice, color_map cm)
{

  bool ok;
  unsigned short* sb = background(wi, slice, ok);
  if(!ok){
    cerr << "ImageBuilder::addBackground didn't obtain background" << endl;
    delete sb;
    return(false);
  }
  
  // modify the current rgbData
  channel_info ci=channels[wi];
  ci.color = cm;
  toRGB(sb, rgbData, ci, data->pwidth() * data->pheight() );
  delete(sb);
  setRGBImage(rgbData, data->pwidth(), data->pheight());
  return(true);
}

bool ImageBuilder::subBackground(unsigned int wi, unsigned int slice){
  bool ok;
  unsigned short* sb = background(wi, slice, ok);
  if(!ok){
    cerr << "ImageBuilder::subBackground didn't obtain background" << endl;
    delete sb;
    return(false);
  }
  channel_info ci=channels[wi];
  ci.color.r = -ci.color.r;
  ci.color.g = -ci.color.g;
  ci.color.b = -ci.color.b;
  toRGB( sb, rgbData, ci, data->pwidth() * data->pheight() );
  delete(sb);
  setRGBImage(rgbData, data->pwidth(), data->pheight());
  return(true);
}

// Estimates the backround spectral response, and sets the values in bg_spectrum.
void ImageBuilder::paintSpectralResponse(set<unsigned int> wi, unsigned int slice)
{
  // let's make sure all channels exist
  vector<unsigned int> ch;
  for(set<unsigned int>::iterator it=wi.begin(); it != wi.end(); ++it){
    if((*it) < channels.size())
      ch.push_back(*it);
  }
  if(!ch.size()){
    cerr << "ImageBuilder paintSpectralResponse, empty channel set" << endl;
    return;
  }
  float** images = getFloatImages(ch, slice);
  // float** images = new float*[ch.size()];
  // for(uint i=0; i < ch.size(); ++i){
  //   images[i] = new float[ data->pwidth() * data->pheight() ];
  //   memset((void*)images[i], 0, sizeof(float) * data->pwidth() * data->pheight());
  //   data->readToFloat(images[i], 0, 0, slice, data->pwidth(), data->pheight(), 1, ch[i]);
  // }
  SpectralResponse spr;
  float** bgr = spr.bg_response(images, ch.size(), data->pwidth(), data->pheight());
  if(!bgr){
    for(uint i=0; i < ch.size(); ++i)
      delete []images[i];
    delete []images;
    return;
  }

  // Then we need to paint the bgr in some way. But the question is what parameters
  // to use. The maximum background response will be close to 1, and that is likely
  // to be true for the green colors, With much lower values elsewhere.
  // we could autoscale the response, by remembering the max values.
  // Still, I would suggest using the channel thingies.
  memset((void*)rgbData, 0, sizeof(float) * 3 * data->pwidth() * data->pheight());

  bg_spectrum.clear();
  for(uint i=0; i < ch.size(); ++i){
    toRGB(bgr[i], rgbData, channels[ch[i]], data->pwidth() * data->pheight());
    distributions[ch[i]]->setData(bgr[i], data->pwidth() * data->pheight(), 0, 1.0, true);
    bg_spectrum[ch[i]] = mode_average(bgr[i], data->pwidth() * data->pheight(), 500, 0, 1.0);
    cout << "Background response for channel " << ch[i] << " : " << bg_spectrum[ch[i]] << endl; 
    delete []bgr[i];
    delete []images[i];
  }
  delete []bgr;
  delete []images;
  setRGBImage(rgbData, data->pwidth(), data->pheight());
  distTabs->show();
}

// ps_ch is a pseduo channel used to specify the colour and the other parameters for turning the
// background into an rgb image
void ImageBuilder::paintSpectralBackground(set<unsigned int> wi, unsigned int slice, unsigned int ps_ch)
{
  if(bg_spectrum.size() < 2){
    cerr << "Unable to build spectrally background image. bg_spectrum too small: " << bg_spectrum.size() << endl;
    return;
  }
  for(map<unsigned int, float>::iterator it=bg_spectrum.begin();
      it != bg_spectrum.end(); it++){
    if(!wi.count(it->first)){
      cerr << "Unable to build spectrally subtracted image. Not enough channels specified : " << it->first << endl;
      return;
    }
  }
  // and select the known things.
  vector<unsigned int> ch;
  vector<float> bgr;
  for(set<unsigned int>::iterator it=wi.begin(); it != wi.end(); ++it){
    if((*it) < channels.size() && bg_spectrum.count(*it)){
      ch.push_back(*it);
      bgr.push_back( bg_spectrum[*it] );
    }
  }
  if(ch.size() < 1){
    cerr << "ImageBuilder::paintSpectralBackground no appropriate channel specified" << endl;
    return;
  }
  if(ps_ch >= channels.size())
    ps_ch = 0;
  
  float** images = getFloatImages(ch, slice);
  float* sp_bg = spectralBackgroundEstimate(images, bgr, bgr.size(), data->pwidth() * data->pheight());
  memset((void*)rgbData, 0, sizeof(float) * 3 * data->pwidth() * data->pheight());
  toRGB(sp_bg, rgbData, channels[ps_ch], data->pwidth() * data->pheight());
  deleteFloats(images, ch.size());
  delete []sp_bg;
  setRGBImage(rgbData, data->pwidth(), data->pheight());
}

bool ImageBuilder::addMCP(unsigned int wi, unsigned int beg, unsigned int end)
{
  if(beg > end){
    unsigned int b = beg;
    beg = end;
    end = b;
  }
  if(wi >= channels.size()){
    cerr << "ImageBuilder::addMCP specified channel too large: " << wi << endl;
    return(false);
  }
  // We need two short buffers:
  unsigned short* contrast_projection = buildShortMCPProjection(sBuffer, wi, beg, end);
  toRGB(contrast_projection, rgbData, channels[wi], data->pwidth() * data->pheight());
  setRGBImage(rgbData, data->pwidth(), data->pheight());
  return(true);
}

bool ImageBuilder::setColor(unsigned int wi, float r, float g, float b)
{
  if(wi >= channels.size()){
    cerr << "ImageBuilder::setColor waveIndex is larger than channel size: " << wi << " >= " << channels.size() << endl;
    return(false);
  }
  channels[wi].color = color_map(r, g, b);
  return(true);
}

bool ImageBuilder::setScaleAndBias(unsigned int wi, float b, float s)
{
  if(wi >= channels.size()){
    cerr << "ImageBuilder::setScaleAndBias waveIndex is larger than channel size: " << wi << " >= " << channels.size() << endl;
    return(false);
  }
  channels[wi].bias = b;
  channels[wi].scale = s;
  return(true);
}

bool ImageBuilder::setMaxLevel(unsigned int wi, float ml)
{
  if(wi >= channels.size()){
    cerr << "ImageBuilder::setMaxLevel waveIndex is larger than channel size: " << wi << " >= " << channels.size() << endl;
    return(false);
  }
  channels[wi].maxLevel = ml;
  return(true);
}

bool ImageBuilder::setBackgroundPars(unsigned int wi, int cw, int ch, int cd, float qnt, bool sub)
{
  if(wi >= channels.size()){
    cerr << "ImageBuilder::setBackgroundPars waveIndex is larger than channel size: "
	 << wi << " >= " << channels.size() << endl;
    return(false);
  }
  channels[wi].bgPar = backgroundPars(cw, ch, cd, qnt);
  channels[wi].bg_subtract = sub;
  return(true);
}

// makes it dark.. 
void ImageBuilder::resetRGB(){
  memset(rgbData, 0, sizeof(float) * data->pwidth() * data->pheight() * 3);
  setRGBImage(rgbData, data->pwidth(), data->pheight());
}

void ImageBuilder::resetOverlayData()
{
  memset((void*)overlayData, 0, sizeof(unsigned char) * 4 * data->pwidth() * data->pheight());
  image->setBigOverlay(overlayData, 0, 0, data->pwidth(), data->pheight());
  
}

void ImageBuilder::reportParameters(){
  for(uint i=0; i < channels.size(); ++i){
    cout << i << " : " << channels[i].finfo.excitation << " --> " << channels[i].finfo.emission << "\t: " << channels[i].finfo.exposure << endl;
    cout << channels[i].maxLevel << " : " << channels[i].bias << " : " << channels[i].scale << "\trgb: "
	 << channels[i].color.r << "," << channels[i].color.g << "," << channels[i].color.b << endl;
  }
}

void ImageBuilder::exportTiff(QString fname)
{
  TiffReader tr;
  if(!tr.makeFromRGBFloat(rgbData, data->pwidth(), data->pheight())){
    cerr << "ImageBuilder::exportTiff unable to make tiff from RGB" << endl;
    return;
  }
  tr.writeOut(fname.toStdString());
  // Lets make one from the overlay data as well..
  if(!overlayData)
    return;
  TiffReader tr2;
  if(!tr2.makeFromRGBA(overlayData, data->pwidth(), data->pheight())){
    warn("Unable to make overlay tiff reader");
    return;
  }
  tr2.addMerge(&tr);
  QString olFile = QString("ol_") + fname;
  tr2.writeOut(olFile.toStdString());
}

bool ImageBuilder::buildShortProjection(unsigned short* p_buffer, unsigned int wi, unsigned int beg, unsigned int end)
{
  if(beg > end){
    unsigned int b = beg;
    beg = end;
    end = b;
  }
  unsigned short* sb = new unsigned short[data->pwidth() * data->pheight()];
  memset((void*)p_buffer, 0, sizeof(unsigned short) * data->pwidth() * data->pheight());
  bool got_something = false;
  for(unsigned int i=beg; i <= end; ++i){
    memset((void*)sb, 0, sizeof(short) * data->pwidth() * data->pheight());
    if( data->readToShort(sb, 0, 0, i, data->pwidth(), data->pheight(), wi) ){
      got_something = true;
      if(channels[wi].bg_subtract)
	sub_slice_bg(wi, i, sb);
      maximize(p_buffer, sb, data->pwidth() * data->pheight() );
    }
  }
  return(got_something);
}

// buildFloatProjection uses fileSet->toFloat. This means that the biases and background projection
// parameters set for frame will be used. This means that background correction may work better around
// bordering things. It also means that contribMap will be used and we should get a smother edge blending
bool ImageBuilder::buildFloatProjection(float* p_buffer, unsigned int wi, int x, int y, int width, int height,
					unsigned int beg, unsigned int end, bool use_cmap)
{
  cout << "buildFloatProjection : " << wi << ": " << x << "," << y << "  dims: " << width << "," << height << "  " << beg << " --> " << end << endl;
  if(width <= 0 || height <= 0 || x + width > data->pwidth() || y + height > data->pheight())
    return(false);
  if(beg > end){
    unsigned int b = beg;
    beg = end;
    end = b;
  }
  float* buffer = new float[width * height];
  memset((void*)p_buffer, 0, sizeof(float) * width * height);
  bool got_something = false;
  for(unsigned int i=beg; i <= end; ++i){
    memset((void*)buffer, 0, sizeof(float) * width * height);
    if( data->readToFloat(buffer, x, y, i, width, height, 1, wi, use_cmap) ){
      got_something = true;
      maximize(p_buffer, buffer, width * height);
    }
  }
  delete []buffer;
  return(got_something);
}

// Having a separate function seems unnecessary. But the coding might be simpler
unsigned short* ImageBuilder::buildShortMCPProjection(unsigned short* p_buffer, unsigned int wi, unsigned int beg, unsigned int end)
{
  if(beg > end){
    unsigned int b = beg;
    beg = end;
    end = b;
  }
  unsigned short* image_buffer = new unsigned short[data->pwidth() * data->pheight()];
  unsigned short* current_contrast = new unsigned short[data->pwidth() * data->pheight()];
  unsigned short* contrast_projection = new unsigned short[data->pwidth() * data->pheight()];
  memset((void*)p_buffer, 0, sizeof(unsigned short) * data->pwidth() * data->pheight());
  memset((void*)contrast_projection, 0, sizeof(unsigned short) * data->pwidth() * data->pheight());
  memset((void*)current_contrast, 0, sizeof(unsigned short) * data->pwidth() * data->pheight());
  for(unsigned int i=beg; i <= end; ++i){
    memset((void*)image_buffer, 0, sizeof(unsigned short) * data->pwidth() * data->pheight());
    if( data->readToShort(image_buffer, 0, 0, i, data->pwidth(), data->pheight(), wi) ){
      if(channels[wi].bg_subtract)
	sub_slice_bg(wi, i, image_buffer);
      make_contrast(image_buffer, current_contrast, data->pwidth(), data->pheight());
      maximize_by_cc(p_buffer, image_buffer, contrast_projection, current_contrast, data->pwidth() * data->pheight() );
    }
  }
  delete image_buffer;
  delete current_contrast;
  delete contrast_projection;
  //  return(contrast_projection);
  return(p_buffer);
}

float* ImageBuilder::projectStack_f(float* stack, unsigned int w, unsigned int h, unsigned int d)
{
  if(!w || !h || !d)
    return(0);
  float* projection = new float[w * h];
  memset((void*)projection, 0, sizeof(float) * w * h);
  for(uint z=0; z < d; ++z)
    maximize(projection, stack + (w * h * z), (w * h));
  return(projection);
}

void ImageBuilder::maximize(unsigned short* a, unsigned short* b, unsigned long l)
{
  for(unsigned short* d=a; d < (a + l); ++d){
    *d = *d > *b ? *d : *b;
    ++b;
  }
}

void ImageBuilder::maximize(float* a, float* b, unsigned long l)
{
  for(float* d=a; d < (a + l); ++d){
    *d = *d > *b ? *d : *b;
    ++b;
  }
}

//  cc = current contrast, cp=contrast projection, c=current image, projection=image projection. 
void ImageBuilder::maximize_by_cc(unsigned short* projection, unsigned short* ci, 
				  unsigned short* cp, unsigned short* cc, unsigned long l)
{
  for(unsigned short* d=projection; d < (projection + l); ++d){
    // cout << *d << "\t" << *ci << "\t" << *cp << "\t" << *cc << endl;
    if(*cc > *cp){
      *cp = *cc;
      *d = *ci;
    }
    ++cc;
    ++cp;
    ++ci;
  }
}

// I have some doubts as to whether this function will do the correct thing..
// but lets wait and see.
void ImageBuilder::make_contrast(unsigned short* image_data, unsigned short* contrast_data, unsigned int w, unsigned int h)
{
  memset((void*)contrast_data, 0, sizeof(unsigned short)*w*h);
  // cheat a bit. don't set any contrast data for the outermost pixels (always set to 0).
  for(uint y=1; y < (h-1); ++y){
    for(uint x=1; x < (w-1); ++x){
      unsigned short v = image_data[ y * w + x ];
      unsigned short d = 0;
      unsigned short d2 = 0;
      for(int dx=-1; dx <= +1; ++dx){
	for(int dy=-1; dy <= +1; ++dy){
	  d2 = abs(v - image_data[ (dy+y)*w + (x+dx)]);
	  d = d2 > d ? d2 : d;
	}
      }
      contrast_data[ y * w + x] = d;
    }
  }
}

void ImageBuilder::sub_slice_bg(unsigned int wi, unsigned int slice, unsigned short* sb)
{
  // slice could be used if we have a cache of some sorts.
  //bool ok;
  unsigned short* bg = background(sb, data->pwidth(), data->pheight(), channels[wi].bgPar);
  //  unsigned short* bg = background(wi, slice, ok);
  // if(!ok){
  //   cerr << "ImageBuilder::sub_slice_bg didn't obtain background" << endl;
  //   delete bg;
  //   return;
  // }
  for(int i=0; i < data->pwidth() * data->pheight(); ++i)
    sb[i] = sb[i] > bg[i] ? sb[i] - bg[i] : 0;
  delete bg;
}

unsigned short* ImageBuilder::background(unsigned int wi, unsigned int slice, bool& ok)
{  

  unsigned short* sb = new unsigned short[ data->pwidth() * data->pheight()];
  memset((void*)sb, 0, sizeof(short)*data->pwidth() * data->pheight());
  if(wi >= channels.size()){
    cerr << "ImageBuilder::addBackground waveIndex too large: " << wi << endl;
    ok = false;
    return(sb);
  }
  if( !data->readToShort(sb, 0, 0, slice, data->pwidth(), data->pheight(), wi) ){
    cerr << "ImageBuilder::addBackground unable to readToShort" << endl;
    ok = false;
    return(sb);
  }
  Two_D_Background bg;
  backgroundPars& bgp = channels[wi].bgPar;
  bg.setBackground(bgp.pcntile, bgp.x_m, bgp.y_m, data->pwidth(), data->pheight(), sb);
  // reuse sb
  unsigned short* sb_ptr = sb;
  for(int y=0; y < data->pheight(); ++y){
    for(int x=0; x < data->pwidth(); ++x){
      (*sb_ptr) = (unsigned short)bg.bg(x, y);
      ++sb_ptr;
    }
  } 
  ok = true;
  return(sb);
}

unsigned short* ImageBuilder::background(unsigned short* sb, unsigned int w, unsigned int h, backgroundPars& bgp)
{
  if(!w || !h){
    cerr << "ImageBuilder::background, null background specified" << endl;
    return(0);
  }
  
  Two_D_Background bg;
  bg.setBackground(bgp.pcntile, bgp.x_m, bgp.y_m, w, h, sb);
  unsigned short* bground = new unsigned short[ w * h ];
  unsigned short* bgptr = bground;
  for(unsigned int y=0; y < h; ++y){
    for(unsigned int x=0; x < w; ++x){
      (*bgptr) = bg.bg(x, y);
      ++bgptr;
    }
  }
  return(bground);
}

void ImageBuilder::toRGB(unsigned short* sb, float* rgb, channel_info& ci, unsigned long l)
{
  float v;
  for(unsigned short* s=sb; s < (sb + l); ++s){
    v = ci.bias + ci.scale * ((float)(*s) / ci.maxLevel);
    if(v > 0){
      *rgb += (v * ci.color.r);
      *(rgb + 1) += (v * ci.color.g);
      *(rgb + 2) += (v * ci.color.b);
      // don't care about going too high. open GL takes care. I believe..
    }
    rgb += 3;
  }
}

void ImageBuilder::toRGB(float* fb, float* rgb, channel_info& ci, unsigned long l)
{
  float v;
  for(float* f=fb; f < (fb + l); ++f){
    v = ci.bias + ci.scale * (*f);
    if(v > 0){
      *rgb += (v * ci.color.r);
      *(rgb + 1) += (v * ci.color.g);
      *(rgb + 2) += (v * ci.color.b);
    }
    rgb += 3;
  }
}

// assumes that rgb has the same dimesions as rgbData. (i.e. data->pwidth() * data->pheight())
void ImageBuilder::toRGB(float* fb, float* rgb, channel_info& ci, unsigned long l,
			 int x_off, int y_off, int width, int height, bool clear,
			 bool use_max_level)
{
  // In any case if clear is true, then lets clear..class ImageAnalyser;

  if(clear)
    memset((void*)rgbData, 0, sizeof(float) * data->pwidth() * data->pheight() * 3);
  // the rgbData defaults to the size of data->pwidth() * data->pheight() 
  // make sure that the x_off + y_off etc don't cause problems.
  // for argument's sake we do allow negative offsets, but these are considered off
  // the image and will cause a shift. See below for how to handle.
  if(x_off < 0){
    width += x_off;
    x_off = 0;
  }
  if(y_off < 0){
    height += y_off;
    y_off = 0;
  }
  if(width <= 0 || height <= 0){
    cerr << "ImageBuilder::toRGB negative width or height specified : " << width << " x " << height << endl;
    return;
  }
  if(x_off >= data->pwidth() || y_off >= data->pheight()){
    cerr << "ImageBilder::toRGB coordinates outside of image: " << x_off << "," << y_off << endl;
    return;
  }
  // make sure width + x_off not bigger than data->pwidth() and so on..
  width = (x_off + width) < data->pwidth() ? width : (data->pwidth() - x_off);
  height = (y_off + height) < data->pheight() ? height : (data->pheight() - y_off);
  
  // and then line by line.. 
  float v;
  float ml = use_max_level ? ci.maxLevel : 1.0;
  for(int dy=0; dy <  height; ++dy){
    float* src = (fb + dy * width);
    float* dst = rgb + 3 * (((y_off + dy) * data->pwidth()) + x_off);
    for(int dx = 0; dx < width; ++dx){
      v = ci.bias + ci.scale * (*src)/ml;
      if(v > 0){
	*dst += (v * ci.color.r);
	*(dst + 1) += (v * ci.color.g);
	*(dst + 2) += (v * ci.color.b);
      }
      ++src;
      dst += 3;
    }
  }
}

bool ImageBuilder::clearRGBSubRect(float* fb, uint fw, uint fh, uint cx, uint cy, 
				uint c_width, uint c_height)
{
  if( cx >= fw || cy >= fh )
    return(false);
  c_width = (cx + c_width) <= fw ? c_width : (fw - cx);
  c_height = (cy + c_height) <= fh ? c_height : (fh - cy);
  for(uint y=cy; y < (cy + c_height); ++y)
    memset((void*)(fb + 3*(y*fw + cx)), 0, sizeof(float) * 3 * c_width);
  return(true);
}

void ImageBuilder::setRGBImage(float* img, unsigned int width, unsigned int height)
{
  int row = 0;
  int col = 0;
  float* subImg = new float[texture_size * texture_size * 3];
  if(!image->isVisible())
    image->show();
  while(row * texture_size < height){
    col = 0;
    while(col * texture_size < width){
      subImage(img, subImg, 3*width, height, 
	       3*col * texture_size, row * texture_size,
	       3*texture_size, texture_size);
      //// We may need to do something a bit more clever than use texture_size
      //// below. If we don't have an exact fit we'll get some quite funny repeats.
      image->setImage(subImg, texture_size, texture_size, col, row);
      ++col;
    }
    ++row;
  }
  delete []subImg;
  image->updateGL();
}


// source_x is the position of the source image
// image is in float rgb format.. 
void ImageBuilder::setBigSubImage(float* img, int source_x, int source_y, 
				  int source_width, int source_height,
				  int source_sub_x, int source_sub_y,
				  int cp_width, int cp_height)
{
  if(cp_width <= 0 || cp_height <= 0 || source_sub_x < 0 || source_sub_y < 0)
    return;
  if(source_sub_x + cp_width >= source_width || source_sub_y + cp_height >= source_height)
    return;
  float* sub_image = new float[ 3 * cp_width * cp_height ];
  for(int y=0; y < cp_height; ++y){
    memcpy( (void*)(sub_image + 3 * (y * cp_width)), 
	    (void*)(img + 3 * ((y + source_sub_y) * source_width + source_sub_x) ),
	    sizeof(float) * cp_width * 3);
  }
  setBigImage(sub_image, source_x + source_sub_x, source_y + source_sub_y, cp_width , cp_height);
  delete []sub_image;
}

void ImageBuilder::setBigSubOverlay(unsigned char* img, int source_x, int source_y,
				    int source_width, int source_height,
				    int source_sub_x, int source_sub_y,
				    int cp_width, int cp_height)
{
  if(cp_width <= 0 || cp_height <= 0 || source_sub_x < 0 || source_sub_y < 0)
    return;
  if(source_sub_x >= source_width || source_sub_y >= source_height){
    return;
  }
  cp_width = source_sub_x + cp_width <= source_width ? cp_width : (source_width - source_sub_x);
  cp_height = source_sub_y + cp_height <= source_height ? cp_height : (source_height - source_sub_y);

  unsigned char* sub_image = new unsigned char[ 4 * cp_width * cp_height ];
  for(int y=0; y < cp_height; ++y){
    memcpy( (void*)(sub_image + 4 * (y * cp_width)), 
	    (void*)(img + 4 * ((y + source_sub_y) * source_width + source_sub_x) ),
	    sizeof(unsigned char) * cp_width * 4);
  }
  setBigOverlay(sub_image, source_x + source_sub_x, source_y + source_sub_y, cp_width , cp_height);
  delete []sub_image;
}

void ImageBuilder::setBigImage(float* img, int source_x, int source_y, int width, int height){
  if(!image->isVisible())
    image->show();
  image->setBigImage(img, source_x, source_y, width, height);
  image->updateGL();
}


void ImageBuilder::setBigOverlay(unsigned char* img, int source_x, int source_y, int width, int height,
				 GLImage::OverlayLayer layer)
{
  if(!image->isVisible())
    image->show();
  image->setBigOverlay(img, source_x, source_y, width, height, layer);
  image->updateGL();
}

// makes an rgba char image (as that is what image expects)
// and sends this off to image thingy. returns the rgba uchar
// overlay image
unsigned char* ImageBuilder::setBigGrayOverlay(float* img, int source_x, int source_y, 
				     int width, int height, unsigned char alpha, float scale)
{
  if(!width || !height || !img)
    return(0);
  if(!image->isVisible())
    image->show();
  int l = width * height;
  unsigned char* olay = new unsigned char[4 * l];
  unsigned char* ol_ptr = olay;
  for(int i=0; i < l; ++i){
    memset((void*)ol_ptr, (unsigned char)(img[i] * scale), 3);
    ol_ptr[3] = alpha;
    ol_ptr += 4;
  }
  image->setBigOverlay(olay, source_x, source_y, width, height);
  image->updateGL();
  return(olay);
}

void ImageBuilder::pipe_slice(std::vector<p_parameter> pars, unsigned int wi, bool reset_rgb){
  if(wi < channels.size())
    pipe_slice(pars, channels[wi], reset_rgb);
}

void ImageBuilder::pipe_slice(vector<p_parameter> pars, channel_info& ch_info, bool reset_rgb)
{
  if(!pars.size())
    return;
  if(reset_rgb)
    resetRGB();
  ps_function func = 0;
  float* img = 0;
  for(uint i=0; i < pars.size(); ++i){
    if(!pipe_slice_functions.count(pars[i].function)){
      cerr << "ImageBuilder::pipe_slice no function defined for : " << pars[i].function << endl;
      break;
    }
    func = pipe_slice_functions[pars[i].function];
    // transfer the image from the last one if we have it
    if(i){
      pars[i].setImage( pars[i-1].image() );
      pars[i].setArea( pars[i-1] );   // Hack to make sure we don't screw up the image that we're passing around
    }
    if(!(*this.*func)(pars[i])){
      cerr << "imageBuilder::pipe_slice func returned false breaking off at i: " << i << endl;
      break;
    }
    img = pars[i].image();
  }
  if(!img){
    cerr << "ImageBuilder::pipe_slice failed to get an image, doing nothing" << endl;
    return;
  }
  //////// note that toRGB will need to be rewritten to handle sub images;
  //////// Since the pipe functions will happily extract any subset of the
  //////// the overall image. 
  cout << "calling toRGB with : " << (long)img << " dims : " << pars[0].width << "," << pars[0].height << "  pos: " 
       << pars[0].x << "," << pars[0].y << endl;
  toRGB(img, rgbData, ch_info, pars[0].width * pars[0].height, pars[0].x, pars[0].y, pars[0].width, pars[0].height);
  setRGBImage(rgbData, data->pwidth(), data->pheight());
  delete []img;
}

void ImageBuilder::general_command(f_parameter& par){
  if(!general_functions.count(par.function())){
    cerr << "ImageBuilder::general_command, unknown command : " << par.function().toAscii().constData() << endl;
    return;
  }
  cout << "ImageBuilder::general command function name is : a QString " << par.function().toAscii().constData() << endl;
  g_function func = general_functions[par.function()];
  (*this.*func)(par);
}

void ImageBuilder::getStackStats(unsigned int wi, int xb, int yb, int zb, int s_width, int s_height, int s_depth)
{
  if(wi >= channels.size()){
    cerr << "ImageBuilder::getStackStats wi is larger than channels.size()" << endl;
    return;
  }
  int col_no, row_no, pwidth, pheight;
  col_no = row_no = pwidth = pheight = 0;
  data->stackDimensions(col_no, row_no, pwidth, pheight);
  
  if(xb + s_width > pwidth || yb + s_height > pheight){
    cerr << "ImageBuilder::getStackStats illegal parameters" << endl;
    return;
  }
  vector<stack_stats> all_stats;
  for(int col=0; col < col_no; ++col){
    for(int row=0; row < row_no; ++row){
      all_stats.push_back(data->stackStats(col, row, xb, yb, zb, s_width, s_height, s_depth, wi));
      //      stack_stats stats = data->stackStats(col, row, xb, yb, zb, s_width, s_height, s_depth, wi);
      // and let's just print out some stats..
      //      cout << row << "," << col << " : "  << wi 
      //   << "\t" << stats.minimum << " -> " << stats.maximum << "  mean: " << stats.mean << "  median : " << stats.median 
      //   << "  mode : " << stats.mode << endl;
      //for(unsigned int i=0; i < stats.qntiles.size(); ++i)
      //cout << "\t" << stats.qntiles[i];
    }
  }
  unsigned int i=0;
  for(int col=0; col < col_no; ++col){
    for(int row=0; row < row_no; ++row){
      cout << col << "," << row << "\t" << all_stats[i].minimum << "-" << all_stats[i].maximum << "\t" 
	   << all_stats[i].mean << "\t" << all_stats[i].median << "\t" << all_stats[i].mode;
      for(uint j=0; j < all_stats[i].qntiles.size(); ++j)
	cout << "\t" << all_stats[i].qntiles[j];
      cout << endl;
      ++i;
    }
  }
}

// first two words have already been taken. Start looking from words[2] for
// further instructions..
QString ImageBuilder::help(std::vector<QString>& words)
{
  QString helpString;
  QTextStream qts(&helpString);
  if(words.size() < 3 || !(pipe_slice_functions.count(words[2].toAscii().constData()) || general_functions.count(words[2])) )
    return( generateGeneralHelp() );
  if(pipe_slice_functions.count(words[2].toAscii().constData()))
    return( generatePipeSliceHelp(words[2]));
  return( generateGeneralFunctionHelp(words[2]) );
  
}

void ImageBuilder::subImage(float* source, float* dest,
			    unsigned int s_width, unsigned int s_height,
			    unsigned int s_x, unsigned int s_y,
			    unsigned int d_width, unsigned int d_height
			    )
{
  memset((void*)dest, 0, sizeof(float)*d_height*d_width);
  float* source_beg = source + (s_y * s_width) + s_x;
  unsigned int copy_width = (s_x + d_width <= s_width) ? d_width : (s_width - s_x);
  for(unsigned int y=0; y < d_height && (s_y + y < s_height); ++y)
    memcpy((void*)(dest + d_width * y), (const void*)(source_beg + y * s_width), sizeof(float) * copy_width);
  
}

// images should contain only those images for which we have a background
// response estimate contained in bgr (which must be the same size as images)
float* ImageBuilder::spectralBackgroundEstimate(float** images, vector<float> bgr, unsigned int im_no, unsigned int im_l)
{
  float* bg = new float[im_l];
  
  for(uint i=0; i < im_l; ++i){
    bg[i] = images[0][i] / bgr[0];
    for(uint j=1; j < im_no; ++j)
      bg[i] = (images[j][i] / bgr[j]) < bg[i] ? (images[j][i] / bgr[j]) : bg[i];
  }
  return(bg);
}

float** ImageBuilder::getFloatImages(vector<unsigned int> ch, unsigned int slice, bool use_cmap)
{
  return(getFloatImages(ch, slice, 0, 0, data->pwidth(), data->pheight(), use_cmap));
}

float** ImageBuilder::getFloatImages(vector<unsigned int> ch, unsigned int slice, int x, int y, int w, int h, bool use_cmap)
{
  if(!ch.size()){
    cerr << "ImageBuilder::getFloatImages no channels specified. " << endl;
    return(0);
  }
  // make sure w and h are positive and non-negative 
  if(w <= 0 || h <= 0)
    return(0);

  float** images = new float*[ch.size()];
  for(uint i=0; i < ch.size(); ++i){
    images[i] = new float[ w * h ];
    memset((void*)images[i], 0, sizeof(float) * w * h);
    data->readToFloat(images[i], x, y, slice, w, h, 1, ch[i], use_cmap);
  }
  return(images);
}

void ImageBuilder::deleteFloats(float** fl, unsigned int l){
  for(uint i=0; i < l; ++i)
    delete []fl[i];
  delete fl;
}

void ImageBuilder::toMean(float* mean, unsigned short* add, int l, float fraction){
  for(int i=0; i < l; ++i)
    mean[i] += (fraction * ((float)add[i]));
}

// sets circumference to 1.0, and adds 0.25 to internal part of the circle.
void ImageBuilder::drawCircle(float* image, int imWidth, int imHeight, int x, int y, int r){
  if(imWidth < 0 || imWidth < 0 || r < 0 || x < 0 || y < 0)
    return;
  int yb = y >= r ? r : y;
  int ye = (y + r) <= imHeight ? r : (imHeight - y) - 1;
  int xb = x >= r ? r : x;
  int xe = (x + r) <= imWidth ? r : (imWidth - x) - 1;
  for(int dy=-yb; dy <= ye; ++dy){
    for(int dx=-xb; dx <= xe; ++dx){
      float d = sqrt( (dy * dy) + (dx * dx) );
      if(d > (float)r)
	continue;
      if(((float)r - d) <= 1.0){
	image[ (y + dy) * imWidth + (x + dx) ] = 1.0;
	continue;
      }
      image[ (y + dy) * imWidth + (x + dx) ] += 0.25;
    }
  }
} 

// draw outlines on RGB-data...
//void ImageBuilder::project_blobs(Blob_mt_mapper* bmapper, float r, float g, float b)
void ImageBuilder::project_blobs(Blob_mt_mapper* bmapper, qnt_colors& qntColor, unsigned char alpha)
{
  int x, y, z;
  uint w, h, d;
  bmapper->position(x, y, z);
  bmapper->dims(w, h, d);
  
  unsigned char r,g,b;
  QString color_par = qntColor.par;
  
  int imWidth = data->pwidth();
  int imHeight = data->pheight();

  // may not actually need the thingy.
  vector<blob*> blobs = bmapper->rblobs();
  for(uint i=0; i < blobs.size(); ++i){
    // obtain the color from the qntColor struct.. 
    qntColor.setColor( getBlobParameter(blobs[i], color_par), r, g, b );
    uint peakSlice = blobs[i]->peakPos / (w * h);
    int px = x + (blobs[i]->peakPos % w);
    int py = y + ((blobs[i]->peakPos % (w * h)) / w);
    if(px > 0 && px < imWidth && py > 0 && py < imHeight){
      int off = 4 * (py * imWidth + px);
      overlayData[off] = r;
      overlayData[off + 1] = g;
      overlayData[off + 2] = b;
      overlayData[off + 3] = alpha;
    }
    for(uint j=0; j < blobs[i]->points.size(); ++j){
      if(blobs[i]->surface[j] && blobs[i]->points[j] / (w * h) == peakSlice){
    	px = x + (blobs[i]->points[j] % w);
    	py = y + ((blobs[i]->points[j] % (w * h)) / w);
    	if(px > 0 && px < imWidth && py > 0 && py < imHeight){
    	  int off = 4 * (py * imWidth + px);
    	  overlayData[off] = r;
    	  overlayData[off + 1] = g;
    	  overlayData[off + 2] = b;
	  overlayData[off + 3] = alpha;
	  //    	  rgbData[off] += 0.5;
    	  //rgbData[off + 1] += 0.5;
    	  //rgbData[off + 2] += 0.5;
    	}
      }
    }
  }
  // and then simply set the thingy.. 
  //  setRGBImage(rgbData, data->pwidth(), data->pheight());
  setBigSubOverlay(overlayData, 0, 0, data->pwidth(), data->pheight(), x, y, w, h);
}

void ImageBuilder::project_blob(blob* b, stack_info& pos, QColor& color)
{
  unsigned char red, green, blue, alpha;
  red = color.red();
  green = color.green();
  blue = color.blue();
  alpha = color.alpha();
  int imWidth = data->pwidth();
  int px, py;
  unsigned int peak_slice = b->peakPos / (pos.w * pos.h);
  // first a point for the peak.
  px = pos.x + b->peakPos % pos.w;
  py = pos.y + ((b->peakPos % (pos.w * pos.h)) / pos.w );
  unsigned int off = 4 * (py * imWidth + px);
  overlayData[off] = red;
  overlayData[off + 1] = green;
  overlayData[off + 2] = blue;
  overlayData[off + 3] = alpha;
  for(uint i=0; i < b->points.size(); ++i){
    if(b->surface[i] && b->points[i] / (pos.w * pos.h) == peak_slice){
      px = pos.x + (b->points[i] % pos.w);
      py = pos.y + ((b->points[i] % (pos.w * pos.h)) / pos.w);
      off = 4 * (py * imWidth + px);
      overlayData[off] = red;
      overlayData[off + 1] = green;
      overlayData[off + 2] = blue;
      overlayData[off + 3] = alpha;
    }
  }
}

void ImageBuilder::updateStacks(QString& name, ImStack* stack, bool add)
{
  if(!add && !imageStacks.count(name))
    return;
  QString description;
  QTextStream qts(&description);
  qts << name << " : " << stack->description().c_str();
  if(add){
    if(imageStacks.count(name)){
      delete imageStacks[name];
      imageStacks.erase(name);
      emit stackDeleted(description);
    }
    imageStacks.insert(make_pair(name, stack));
    emit stackCreated(description);
    return;
  }
  delete imageStacks[name];
  imageStacks.erase(name);
  emit stackDeleted(description);
}

void ImageBuilder::warn(const char* message)
{
  emit displayMessage(message);
}

void ImageBuilder::warn(QString& message)
{
  emit displayMessage(message.toAscii().constData());
}

// A simple function. Returns a float* representation of a single slice
// for a single channel. Image width and height are taken from the appropriate
// fields.
bool ImageBuilder::p_slice(p_parameter& par)
{
  if(!par.width || !par.height)
    return(false);
  int z = 0;
  int ch = 0;
  if(!par.param("z", z) || !par.param("ch", ch))
    return(false);
  float* img = par.image();
  if(!img){
    img = new float[par.width * par.height];
    par.setImage(img);
  }
  memset((void*)img, 0, sizeof(float) * par.width * par.height );
  if(!data->readToFloat(img, par.x, par.y, z, par.width, par.height, 1, (unsigned int)ch)){
    return(false);
  }
  return(true);
}

bool ImageBuilder::p_gaussian(p_parameter& par)
{
  if(!par.image()){
    cerr << "ImageBuilder::p_gaussian no image specified. Returning" << endl;
    return(false);
  }
  // extract the relevant parameters.
  float* img = par.image();
  int radius = 0;
  if(!par.param("radius", radius)){
    cerr << "ImageBuilder::p_gaussian, unable to obtain radius" << endl;
    return(false);
  }
  float* bl_image = gaussian_blur_2d(img, par.width, par.height, (unsigned int)radius);
  par.setImage(bl_image);
  return(true);
}

bool ImageBuilder::pl_gaussian(p_parameter& par)
{
  if(!par.image()){
    cerr << "ImageBuilder::pl_gaussian no image specified" << endl;
    return(false);
  }
  float* img = par.image();
  int radius = 0;
  if(!par.param("radius", radius)){
    cerr << "ImageBuilder::pl_gaussian, unable to obtain radius" << endl;
    return(false);
  }
  float* bl_image = gaussian_blur_1d(img, par.width, par.height, (unsigned int)radius);
  if(bl_image){
    par.setImage(bl_image);
    return(true);
  }
  return(false);
}

bool ImageBuilder::p_sub_bg(p_parameter& par){
  if(!par.image()){
    cerr << "ImageBuilder::p_sub_bg no image specified retunring false" << endl;
    return(false);
  }
  int xr, yr;
  float pcntile = 0;
  if(!par.param("bg_xr", xr) || !par.param("bg_yr", yr) || !par.param("bg_pcnt", pcntile)){
    cerr << "ImageBuilder::p_sub_bg Unable to get background paramters" << endl;
    return(false);
  }
  Two_D_Background bg;
  bg.setBackground(pcntile, (unsigned int)xr, (unsigned int)yr, 
		   (unsigned int)par.width, (unsigned int)par.height, par.image());
  // then go through and subtract..
  float* ptr = par.image();
  float b;
  for(int y=0; y < par.height; ++y){
    for(int x=0; x < par.height; ++x){
      b = bg.bg(x, y);
      (*ptr) = b < (*ptr) ? (*ptr) - b : 0;
      ++ptr;
    }
  }
  return(true);
}

bool ImageBuilder::p_sub_channel(p_parameter& par){
  if(!par.image()){
    cerr << "ImageBuilder::p_sub_channel Image needs to be specified in the image" << endl;
  }
  // This function makes the assumption that the img data specified will have the same
  // dimensions. This is sort of enforced by the structure of the calling functions, but
  // as it can't be checked it's a bit of a bummer.
  // Parameters required are:
  // channel 
  // multiplier
  // allow_negative values ?
  // optional parameters:
  // x and y offsets.
  if(!par.width || !par.height)
    return(false);
  int ch;
  int al_neg;
  float multiplier;
  int z;
  if(!par.param("ch", ch) || !par.param("al_neg", al_neg) || !par.param("mult", multiplier) || !par.param("z", z)){
    cerr << "ImageBuilder::p_sub_channel insufficent parameters specified" << endl;
    return(false);
  }
  int xo, yo;
  if(!par.param("xo", xo))
    xo = 0;
  if(!par.param("yo", yo))
    yo = 0;
  float* sub_image = new float[par.width * par.height];
  memset((void*)sub_image, 0, sizeof(float) * par.width * par.height);
  if(!data->readToFloat(sub_image, par.x + xo, par.y + yo, z, par.width, par.height, 1, (unsigned int)ch)){
    cerr << "ImageBuilder::p_sub_channel unable to read the data for the subtraction" << endl;
    delete []sub_image;
    return(false);
  }
  // A gaussian blur can be specified by the g_blur parameter, with radius given by the
  // second thdnngy.
  int g_radius = 0;
  float* bl_sub = 0;
  if(par.param("g_blur", g_radius))
    bl_sub = gaussian_blur_2d(sub_image, par.width, par.height, (unsigned int)g_radius);
  if(bl_sub){
    delete []sub_image;
    sub_image = bl_sub;
  }
  // then we can simply subtract sub_image from img..
  float* img = par.image();
  float* end_image = img + (par.width * par.height);
  if(al_neg){
    while(img < end_image){
      (*img) -= (*sub_image) * multiplier;
      ++img;
      ++sub_image;
    }
  }else{
    while(img < end_image){
      (*img) = (*img) > ((*sub_image) * multiplier) ? (*img) - (*sub_image) * multiplier : 0.0;
      ++img;
      ++sub_image;
    }
  }
  return(true);
}

bool ImageBuilder::p_mean_panel(p_parameter& par)
{
  int ch;
  if(!par.param("ch", ch))
    return(false);
  
  int col_begin = 0;
  int row_begin = 0;
  par.param("cb", col_begin);
  par.param("rb", row_begin);

  if((unsigned int)ch >= channels.size()){
    cerr << "ImageBuilder::p_mean_panel channel number too large" << endl;
    return(false);
  }
  // first get the appropriate dimensions..
  int col_no, row_no, pwidth, pheight;
  col_no = row_no = pwidth = pheight = 0;
  int depth = data->sectionNo();
  data->stackDimensions(col_no, row_no, pwidth, pheight);
  if(!col_no || !row_no || !pwidth || !pheight)
    return(false);
  unsigned short* s_buf = new unsigned short[pwidth * pheight];
  float* mean = new float[pwidth * pheight];
  memset((void*)mean, 0, sizeof(float) * pwidth * pheight);
  float fraction = 1.0 / ( (col_no-col_begin) * (row_no - row_begin) * depth);
  for(int col=col_begin; col < col_no; ++col){
    for(int row=row_begin; row < row_no; ++row){
      for(int slice=0; slice < depth; ++slice){
	if(!data->readToShort(s_buf, col, row, slice, ch)){
	    cerr << "Unable to read short for " << col << "," << row << "," << slice << endl;
	    delete []s_buf;
	    delete []mean;
	    return(false);
	}
	toMean(mean, s_buf, pwidth * pheight, fraction);
      }
    }
  }
  // turn short to float using channel information..
  float* mptr = mean;
  for(mptr = mean; mptr < mean + (pwidth * pheight); ++mptr){
    *mptr = channels[ch].bias + channels[ch].scale * ((*mptr) / channels[ch].maxLevel);
  }
  par.setImage(mean);
  par.width = pwidth;
  par.height = pheight;
  par.x = par.y = 0;
  delete []s_buf;
  return(true);
}

void ImageBuilder::setPanelBias(f_parameter& par)
{
  short bias = 0;
  float scale = 1.0;
  int ch = -1;
  int row=-1;
  int col=-1;
  if(!par.param("b", bias) || !par.param("s", scale)){
    cerr << "ImageBuilder::setPanelBias, didn't obtain scale (s) or bias (b); aborting" << endl;
    return;
  }
  if(!par.param("row", row) || !par.param("col", col) || !par.param("ch", ch)){
    cerr << "ImageBuilder::setPanelBias, didn't obtain row (row), column (col) or channel (ch); aborting" << endl;
    return;
  }
  cout << "calling setPanelBias : " << ch << "," << col << "," << row << " : " << scale << "\t" << bias << endl;
  data->setPanelBias((unsigned int)ch, (unsigned int)col, (unsigned int)row, scale, bias);
}

void ImageBuilder::setFrameBackgroundPars(f_parameter& par)
{
  int wi = -1;
  int xm = 0;
  int ym = 0;
  float qnt = 0;
  bool bg_sub = true;
  if(!par.param("wi", wi) || !par.param("xm", xm) || !par.param("ym", ym)
     || !par.param("q", qnt)){
    cerr << "Please specify : wi (int) xm (int) ym (int) q (float)" << endl;
    return;
  }
  par.param("sub", bg_sub);
  data->setBackgroundPars((unsigned int)wi, xm, ym, qnt, bg_sub);
}

void ImageBuilder::addSlice(f_parameter& par){
  vector<unsigned int> ch;
  if(!par.param("ch", ',', ch) && !par.param("wi", ',', ch)){  // allow both ch= and wi=
    cerr << "ImageBuilder::addSlice please specify the channels ch=1,2,3 or wi=1,2,3 etc.." << endl;
    return;
  }
  // we better check we don't have any stupid channels..
  vector<unsigned int> wi;
  wi.reserve(ch.size());
  for(uint i=0; i < ch.size(); ++i){
    if(ch[i] < channels.size())
      wi.push_back(ch[i]);
  }
  if(!wi.size())
    return;
  unsigned int z;
  if(!par.param("z", z)){
    warn("ImageBuilcer::addSlice please specify the z position\n");
    return;
  }
  int x = 0;
  int y = 0;
  int w = data->pwidth();
  int h = data->pheight();
  // allow overwriting of these..
  par.param("x", x); par.param("y", y); par.param("w", w); par.param("h", h);
  if( (x < 0 || y < 0 || w <=0 || h <= 0) ){
    cerr << "ImageBuilder::addSlice please don't specify negative positions or dimensions" << endl;
    return;
  }
  x = (x >= data->pwidth()) ? 0 : x;
  y = (y >= data->pheight()) ? 0 : y;
  w = (x + w) > data->pwidth() ? (data->pwidth() - x) : w;
  h = (y + h) > data->pheight() ? (data->pheight() - y) : h;
  // and then get the floats using the appropriate function..
  bool use_cmap = true;  // whether or not we use the contribution map
  par.param("cmap", use_cmap);  // optional parameter
 
  float** images = getFloatImages(wi, z, x, y, w, h, use_cmap);
  for(uint i=0; i < wi.size(); ++i)
    toRGB(images[i], rgbData, channels[wi[i]], (w * h), x, y, w, h, false);
  deleteFloats(images, wi.size());
  setRGBImage(rgbData, data->pwidth(), data->pheight());
  
}

void ImageBuilder::blurStack(f_parameter& par)
{
  QString stackName;
  if(!par.param("stack", stackName) || !imageStacks.count(stackName)){
    cerr << "ImageBuilder::blurStack stack not specified or unknown : " << stackName.toAscii().constData() << endl;
    return;
  }
  ImStack* imStack = imageStacks[stackName];
  unsigned int wi=0;
  par.param("wi", wi);
  unsigned int radius = 4;
  if(!par.param("r", radius) && !par.param("radius", radius))
    cerr << "No radius specified, defaulting to a radius of " << radius << endl;
  QString blurStack;
  bool addNewStack;
  addNewStack = par.param("out", blurStack);
  float* stack = imStack->stack(wi);
  if(!stack){
    cerr << "ImageBuilder::blurStack, unable to obtain stack from " << stackName.toAscii().constData() << " (" << wi << ")" << endl;
    return;
  }
  float* bl_stack = gaussian_blur_3d(stack, imStack->w(), imStack->h(), imStack->d(), radius);
  if(!bl_stack){
    cerr << "ImageBuilder::blurStack did not receive a blurred stack from gaussian function" << endl;
    return;
  }
  if(!addNewStack){
    if(!imStack->setData(bl_stack, wi)){
      delete []bl_stack;
    }
    return;
  }
  // in this case we want to make a new image stack..
  channel_info cinfo = imStack->cinfo(wi);
  ImStack* newStack = new ImStack(bl_stack, cinfo, imStack->xp(), imStack->yp(), imStack->zp(),
				  imStack->w(), imStack->h(), imStack->d());
  updateStacks(blurStack, newStack, true);
}

// seems like a imStack* getStack(f_paramter& par) function would be useful, 
void ImageBuilder::stack_mult_z_nbor(f_parameter& par)
{
  QString stackName;
  if(!par.param("stack", stackName) || !imageStacks.count(stackName)){
    cerr << "ImageBuilder::blurStack stack not specified or unknown : " << stackName.toAscii().constData() << endl;
    return;
  }
  QString newStackName;
  if(!par.param("m_stack", newStackName)){
    warn("Please specify the name of the new multiplied stack m_stack=..");
    return;
  }
  ImStack* imStack = imageStacks[stackName];
  unsigned int wi=0;
  par.param("wi", wi);
  float* stack = imStack->stack(wi);
  if(!stack){
    warn("ImageBuilder::stack_mult_z_nbor wave index specified is probably too large");
    return;
  }
  unsigned long s_size = imStack->w() * imStack->h() * imStack->d();
  if(!s_size){
    warn("Specifed image Stack appears to have null dimensions");
    return;
  }
  float* nStack = new float[ s_size ];
  memset((void*)nStack, 0, sizeof(float) * s_size);
  unsigned int s_area = imStack->w() * imStack->h();
  float* dst = nStack + s_area;  // first and last slices are set to 0.
  float* src_p = stack;
  float* src_c = stack + s_area;
  float* src_a = stack + (s_area * 2);
  float* src_end = stack + s_size;
  while(src_a < src_end){
    *(dst++) = *(src_p++) * *(src_c++) * *(src_a++);
  }
  channel_info c_info = imStack->cinfo(wi);
  ImStack* newStack = new ImStack(nStack, c_info, imStack->xp(), imStack->yp(), imStack->zp(),
				  imStack->w(), imStack->h(), imStack->d());
  updateStacks(newStackName, newStack, true);
}  

void ImageBuilder::blurStack_2d(f_parameter& par)
{
  warn("ImageBuilder::blurStack_2d has been put on the backshelf for now");
  return;
  QString stackName;
  if(!par.param("stack", stackName) || !imageStacks.count(stackName)){
    warn("blurStack_2d no stack=.. specified or unknown stack");
    return;
  }
  ImStack* stack = imageStacks[stackName];
  std::vector<unsigned int> stack_channels;
  for(unsigned int i=0; i < stack->ch(); ++i)  // default to blurring all channels
    stack_channels.push_back(i);
  if(par.param("ch", ',', stack_channels)){
    for(unsigned int i=0; i < stack_channels.size(); ++i){
      if(stack_channels[i] >= stack->ch()){
	warn("blurStack_2d stack_channel too large");
	return;
      }
    }
  }
  unsigned int radius = 4;
  par.param("r", radius);
  if(!radius){
    warn("blurStack_2d illegal (0) radius specified");
    return;
  }
  
}

void ImageBuilder::deBlurStack(f_parameter& par)
{
  QString stackName;
  if(!par.param("stack", stackName) || !imageStacks.count(stackName)){
    cerr << "ImageBuilder::deBlurStack stack not specified or unknown : " << stackName.toAscii().constData() << endl;
    return;
  }
  ImStack* imStack = imageStacks[stackName];
  unsigned int wi=0;
  par.param("wi", wi);
  unsigned int radius = 4;
  if(!par.param("r", radius) && !par.param("radius", radius))
    cerr << "No radius specified, defaulting to a radius of " << radius << endl;
  QString blurStack;
  bool addNewStack;
  addNewStack = par.param("out", blurStack);
  float* stack = imStack->stack(wi);
  if(!stack){
    cerr << "ImageBuilder::blurStack, unable to obtain stack from " << stackName.toAscii().constData() << " (" << wi << ")" << endl;
    return;
  }
  float* bl_stack = gaussian_deblur_3d(stack, imStack->w(), imStack->h(), imStack->d(), radius);
  if(!bl_stack){
    cerr << "ImageBuilder::blurStack did not receive a blurred stack from gaussian function" << endl;
    return;
  }
  if(!addNewStack){
    if(!imStack->setData(bl_stack, wi)){
      delete []bl_stack;
    }
    return;
  }
  // in this case we want to make a new image stack..
  channel_info cinfo = imStack->cinfo(wi);
  ImStack* newStack = new ImStack(bl_stack, cinfo, imStack->xp(), imStack->yp(), imStack->zp(),
				  imStack->w(), imStack->h(), imStack->d());
  updateStacks(blurStack, newStack, true);
}

void ImageBuilder::subStack(f_parameter& par)
{
  vector<QString> stacks;
  par.param("stacks", ',', stacks);
  if(stacks.size() < 2){
    cerr << "subStack at least two stacks need to be specified for a -= b do: stacks=a,b ; c = a - b : stacks=a,b,c" << endl;
    return;
  }
  if(!imageStacks.count(stacks[0]) || !imageStacks.count(stacks[1])){
    cerr << "At least one specified stack is unknown" << endl;
    return;
  }
  // equation represented by 
  // a = a - b  (if only one image stacks and a is not the same as b and so on.. )
  // c = a - b  (if a third image stack is defined)
  // but c is a copy of a, and still referred to as a.. 
  ImStack* a = imageStacks[stacks[0]];
  ImStack* b = imageStacks[stacks[1]];
  if(stacks.size() > 2 ){
    a = new ImStack(*a);
    updateStacks(stacks[2], a, true);
  }
  // a and b can have many different channels, and could be of different sizes. So we need to 
  // specify something reasonable. for this..
  unsigned int ch_a = 0;
  unsigned int ch_b = 0;
  par.param("ch_a", ch_a);
  par.param("ch_b", ch_b);
  float mult = 1.0;
  par.param("mult", mult);
  bool allowNeg = false;
  par.param("allow_neg", allowNeg);

  int xoff, yoff, zoff;
  xoff = yoff = zoff = 0;
  par.param("xo", xoff);
  par.param("yo", yoff);
  par.param("zo", zoff);

  if(!a->subtract(b, ch_a, ch_b, mult, allowNeg, xoff, yoff, zoff))
    cerr << "subStack, unable to subtract " << stacks[1].toAscii().constData() << " from " << stacks[0].toAscii().constData() << endl;
}

void ImageBuilder::subStackBackground(f_parameter& par)
{
  QString stackName;
  if(!par.param("stack", stackName) || !imageStacks.count(stackName)){
    cerr << "subStackBackground no stack with name : " << stackName.toAscii().constData() << endl;
    return;
  }
  int xm = 12;
  int ym = 12;
  float q=0.1;
  unsigned int wi=0;
  QString outStack;
  bool newStack = par.param("out", outStack);
  // and then.. the other parameters..
  par.param("xm", xm);
  ym = xm;        // allows you to set only xm, and ym will default to the same value
  par.param("ym", ym);
  par.param("wi", wi);
  par.param("q", q);

  ImStack* stack = imageStacks[stackName];
  if(wi >= stack->ch()){
    cerr << "subStackBackground, stack does not have wi of : " << wi << endl;
    return;
  }
  if(newStack){
    stack = new ImStack(*stack);
    updateStacks(outStack, stack, true);
  }
  // go through all of the slices, create a background and then subtract. Very time consuming, but maybe
  // ok as we save the result.. 
  Two_D_Background tdb;
  for(uint zp=stack->zp(); zp < (stack->zp() + stack->d()); ++zp){
    float* data = stack->image(wi, zp);
    if(!data)
      continue;
    cout << "setting background for : " << zp << endl;
    if(!tdb.setBackground(q, xm, ym, (int)stack->w(), (int)stack->h(), data))
      cerr << "Failed to set background for unspecified reason" << endl;
    // and then simply let's subtract..
    cout << "\tsubtracting.." << endl;
    for(int y=0; y < (int)stack->h(); ++y){
      float* dest = data + y * stack->w(); 
      for(int x=0; x < (int)stack->w(); ++x){
	//if(zp == 20 && y == 400 && !(x % 50));
	
	//	cout << zp << "," << y << "," << x << " dest " << *dest << "  bg " << tdb.bg(x,y) << " = " << (*dest) - tdb.bg(x,y) << endl;
	(*dest) -= tdb.bg(x,y);
	(*dest) = (*dest) < 0 ? 0 : (*dest);
	++dest;
      }
    }
  }
}

// Get an averaged imaged of a load of tiles, then find center by finding maximum
// rings.. 
void ImageBuilder::findCenter(f_parameter& par)
{
  int ch;
  if(!par.param("ch", ch))
    return;
  unsigned int wi = (unsigned int)ch;
  int row_begin = 0;
  int col_begin = 0;
  par.param("rb", row_begin);
  par.param("cb", col_begin);
  if(wi >= channels.size())
    return;
  
  int col_no, row_no, pwidth, pheight;
  col_no = row_no = pwidth = pheight = 0;
  //  int depth = data->sectionNo();
  data->stackDimensions(col_no, row_no, pwidth, pheight);
  p_parameter p_par = f_to_p_param(par, pwidth, pheight, 0, 0, 0);
  if(!p_mean_panel(p_par)){
    cerr << "ImageBuilder::findCenter p_mean_panel returned false" << endl;
    return;
  }
  if(!p_par.image()){
    cerr << "ImageBuilder::findCenter p_mean_panel returned true, but no image specified" << endl;
    return;
  }
  float* image = p_par.image();
  // we now need somehow to get a vector of radiuses from par (f_parameter)
  vector<int> radiuses;
  if(!par.param("r", ',', radiuses)){
    cerr << "ImageBuilder::findCenter unable to get radiuses (r=4,20,40,...) : " << endl;
    delete []image;
    return;
  }
  CenterFinder cfinder(image, pwidth, pheight, radiuses);
  vector<cf_circle> circles = cfinder.findCenter();
  // then draw the circles onto the image. Use white colour, or specify something.
  
}

void ImageBuilder::loopStack(f_parameter& par)
{
  QString stackName;
  if(!par.param("stack", stackName)){
    cerr << "ImageBuilder::loopStack no stack name (stack) specified" << endl;
    return;
  }
  if(!imageStacks.count(stackName)){
    cerr << "ImageBuilder::loopStack no stack called : " << stackName.toAscii().constData() << endl;
    return;
  }
  // but don't use yet, as we don't have the ability to clear only a small area..
  bool clearImage = true;
  par.param("clear", clearImage);
  unsigned int n = 1;
  par.param("n", n);
  ImStack* stack = imageStacks[stackName];
  unsigned int msleep = 100;
  par.param("ms", msleep);
  unsigned int zb=0; unsigned int ze=stack->d();
  par.param("zb", zb); par.param("ze", ze);
  ze = ze > stack->d() ? stack->d() : ze;
  for(uint i=0; i < n; ++i){
    for(unsigned int dz=zb; dz < ze; ++dz){
      memset((void*)rgbData, 0, sizeof(float) * data->pwidth() * data->pheight() * 3);
      for(uint ch=0; ch < stack->ch(); ++ch){
	channel_info cinfo = stack->cinfo(ch);
	toRGB( stack->image(ch, dz + stack->zp()), rgbData,
	       cinfo, (unsigned long)(stack->w() * stack->h()), stack->xp(), stack->yp(),
	       (int)stack->w(), (int)stack->h(), false );
      }
      setRGBImage(rgbData, data->pwidth(), data->pheight());
      usleep(msleep * 1000);
    }
  }
}

void ImageBuilder::loopXZ(f_parameter& par)
{
  QString stackName;
  if(!par.param("stack", stackName)){
    cerr << "ImageBuilder::loopStack no stack name (stack) specified" << endl;
    return;
  }
  if(!imageStacks.count(stackName)){
    cerr << "ImageBuilder::loopStack no stack called : " << stackName.toAscii().constData() << endl;
    return;
  }
  // but don't use yet, as we don't have the ability to clear only a small area..
  bool clearImage = true;
  par.param("clear", clearImage);
  ImStack* stack = imageStacks[stackName];
  unsigned int msleep = 10;
  par.param("ms", msleep);
  unsigned int wi = 0;
  par.param("wi", wi);
  wi = wi < stack->ch() ? wi : 0;
  unsigned int yb = 0;
  unsigned int ye = stack->h();
  channel_info cinfo = stack->cinfo(wi);
  for(unsigned int yp=yb; yp < ye; ++yp){
    int slice_width, slice_height;
    float* slice = stack->xz_slice(wi, yp, slice_width, slice_height);
    if(clearImage)
      memset((void*)rgbData, 0, sizeof(float) * data->pwidth() * data->pheight() * 3);
    toRGB( slice, rgbData, cinfo, slice_width * slice_height, stack->xp(), stack->yp() + (stack->h() / 2),
	   slice_width, slice_height, false );
    setRGBImage(rgbData, data->pwidth(), data->pheight());
    delete []slice;
    usleep(msleep * 1000);
  }
}

void ImageBuilder::stack_xzSlice(f_parameter& par)
{
  QString stackName;
  if(!par.param("stack", stackName))
    return;
  if(!imageStacks.count(stackName))
    return;
  ImStack* stack = imageStacks[stackName];
  bool clearImage = true;
  par.param("clear", clearImage);
  unsigned int ypos = stack->w() / 2;
  par.param("yp", ypos);
  unsigned int wi = 0;
  par.param("wi", wi);
  int slice_width, slice_height;
  float* slice;
  if( !( slice = stack->xz_slice(wi, ypos, slice_width, slice_height) )){
    cerr << "ImageBuilder::stack_xzSlice imStack->xz_slice returned false" << endl;
    return;
  }
  if(clearImage)
    memset((void*)rgbData, 0, sizeof(float) * data->pwidth() * data->pheight() * 3);
  channel_info cinfo = stack->cinfo(wi);
  toRGB( slice, rgbData, cinfo, slice_width * slice_height, stack->xp(), 
	 stack->yp() + ypos - slice_height / 2, slice_width, slice_height, false );
  setRGBImage(rgbData, data->pwidth(), data->pheight());
  delete []slice;
}

void ImageBuilder::stack_yzSlice(f_parameter& par)
{
  QString stackName;
  if(!par.param("stack", stackName))
    return;
  if(!imageStacks.count(stackName))
    return;
  ImStack* stack = imageStacks[stackName];
  bool clearImage = true;
  par.param("clear", clearImage);
  unsigned int xpos = stack->h() / 2;
  par.param("xp", xpos);
  unsigned int wi = 0;
  par.param("wi", wi);
  int slice_width, slice_height;
  float* slice;
  if( !( slice = stack->yz_slice(wi, xpos, slice_width, slice_height) )){
    cerr << "ImageBuilder::stack_xzSlice imStack->xz_slice returned false" << endl;
    return;
  }
  if(clearImage)
    memset((void*)rgbData, 0, sizeof(float) * data->pwidth() * data->pheight() * 3);
  channel_info cinfo = stack->cinfo(wi);
  toRGB( slice, rgbData, cinfo, slice_width * slice_height, stack->xp() + xpos - slice_width / 2, 
	 stack->yp(), slice_width, slice_height, false );
  setRGBImage(rgbData, data->pwidth(), data->pheight());
  delete []slice;
}

void ImageBuilder::stack_project(f_parameter& par)
{
  QString stackName;
    if(!par.param("stack", stackName))
    return;
  if(!imageStacks.count(stackName))
    return;
  ImStack* stack = imageStacks[stackName];
  bool clearImage = true;
  par.param("clear", clearImage);
  unsigned int wi=0;
  par.param("wi", wi);
  if(wi >= stack->ch()){
    cerr << "stack_project wave index too large" << endl;
    return;
  }
  unsigned int w = stack->w();
  unsigned int h = stack->h();
  unsigned int d = stack->d();
  int xp = stack->xp();
  int yp = stack->yp();
  int zp = stack->zp();

  par.param("zp", zp);
  par.param("d", d);

  if(zp < 0)
    return;
  // but otherwise don't check.. 
  if(!w || !d)
    return;
  
  bool paintBlobs = false;
  par.param("blobs", paintBlobs);

  float* projection = new float[w * h];
  memset((void*)projection, 0, sizeof(float) * w * h);
  for(int z=zp; z < zp + (int)d; ++z){
    if(stack->image(wi, z)){
      maximize(projection, stack->image(wi, z), w * h);
    }else{
      cerr << "stack didn't return anything" << endl;
    }
  }
  channel_info cinfo = stack->cinfo(wi);
  if(clearImage)
    memset((void*)rgbData, 0, sizeof(float) * data->pwidth() * data->pheight() * 3);
  toRGB(projection, rgbData, cinfo, w * h, xp, yp, w, h, false);

  unsigned char blob_alpha = 125;
  par.param("blob_alpha", blob_alpha);

  if(mtMappers.count(stack) && paintBlobs){
    qnt_colors qntColors;
    for(multimap<ImStack*, Blob_mt_mapper*>::iterator it=mtMappers.lower_bound(stack);
	it != mtMappers.upper_bound(stack); ++it){
      //      project_blobs( (*it).second );
      project_blobs( (*it).second, qntColors, blob_alpha );
    }
  }
  setRGBImage(rgbData, data->pwidth(), data->pheight());
  delete []projection;
}

void ImageBuilder::stack_project_cl(f_parameter& par)
{
  QString errorMessage;
  QTextStream error(&errorMessage);
  QString stackName("");
  if(!par.param("stack", stackName))
    error << "specify the name of the stack: stack=..\n";
  if(!imageStacks.count(stackName))
    error << "Unknown stack \n";
  if(errorMessage.length()){
    warn(errorMessage);
    return;
  }
  // otherwise, just make an object and .. get a new projection
  cout << "Creating an open cl projector" << endl;
  MIPf_cl* projector = new MIPf_cl();
  unsigned int local_item_size = 0;
  if(par.param("local_item_size", local_item_size))
    projector->set_local_item_size(local_item_size);
  
  cout << "Calling projectStack on the projector" << endl;
  ImStack* projection = projector->projectStack( imageStacks[ stackName ] );
  if(!projection){
    delete projector;
    return;
  }
  uint rep = 0;
  par.param("repeat", rep);
  for(uint i=0; i < rep; ++i){
    delete projection;
    cout << "calling projectStack" << endl;
    projection = projector->projectStack( imageStacks[ stackName ] );
    cout << "\t\tRETURNED" << endl;
  }

  // clear the image data..
  memset((void*)rgbData, 0, sizeof(float) * data->pwidth() * data->pheight() * 3);
  for(uint i=0; i < projection->ch(); ++i){
    channel_info cinfo = projection->cinfo(i);
    toRGB(projection->stack(i), rgbData, cinfo, projection->w() * projection->h(),
	  projection->xp(), projection->yp(), projection->w(), projection->h(), false);
  }
  setRGBImage(rgbData, data->pwidth(), data->pheight());
  delete projection;
  delete projector;
  cout << "End of project function" << endl;
}

void ImageBuilder::stack_expand_cl(f_parameter& par)
{
  QString errorMessage;
  QTextStream error(&errorMessage);
  QString stackName("");
  if(!par.param("stack", stackName))
    error << "specify the name of the stack: stack=..\n";
  if(!imageStacks.count(stackName))
    error << "Unknown stack \n";
  unsigned int exp_factor=2;
  par.param("exp_factor", exp_factor);
  if(exp_factor < 2 || exp_factor > 10)
    error << "exp_factor is either too big or too small, consider what you're doing\n";
  if(errorMessage.length()){
    warn(errorMessage);
    return;
  }
  float sigma = 1.0;
  par.param("sigma", sigma);
  ImExpand_cl expander;
  unsigned int local_item_size = 0;
  if(par.param("local_item_size", local_item_size))
    expander.set_local_item_size(local_item_size);
  ImStack* expanded = expander.expandStack(imageStacks[stackName], exp_factor, sigma);
  displayStack(expanded);  // defaults to central position
  QString saveStackName;
  par.param("save", saveStackName);
  if(!stackName.length()){
    delete expanded;
  }else{
    if(imageStacks.count(saveStackName))
      delete imageStacks[saveStackName];
    imageStacks[saveStackName] = expanded;
  }
}

void ImageBuilder::setStackPar(f_parameter& par)
{
  unsigned int wi = 0;
  QString param;
  QString stackName;
  if(!par.param("par", param) || !par.param("stack", stackName))
    return;
  par.param("wi", wi);
  if(!imageStacks.count(stackName))
    return;
  if(param == "sandb"){
    float scale, bias;
    if(!par.param("s", scale) || !par.param("b", bias))
      return;
    imageStacks[stackName]->set_sandb(wi, scale, bias);
  }
}

// temporary function. We don't really want to keep this one here
// as we'll need more complex things..
void ImageBuilder::stack_map_blobs(f_parameter& par)
{
  QString stackName;
  QString errorMessage("");
  QTextStream error(&errorMessage);
  if(!par.param("stack", stackName) || !imageStacks.count(stackName)){
    //error << "ImageBuilder::stack_map_blobs No stack with name will try map_blobs() " << stackName.toAscii().constData(); // << "\n";
    //warn(errorMessage);
    map_blobs(par);
    return;
  }
  float minPeak, minEdge;
  if(!par.param("peak", minPeak) || !par.param("edge", minEdge))
    error << "ImageBuilder::stack_map_blobs need to specify peak and edge\n";
  if(minEdge >= minPeak)
    error << "ImageBuilder::stack_map_blobs minEdge should not be more than minPeak\n";

  unsigned int mapper_id = 0;
  if(!par.param("id", mapper_id))
    error << "ImageBuilder::stack_map_blobs it is necessary to provide a mapper id (id=) (which should be a power of 2)\n";

  if(errorMessage.length()){
    warn(errorMessage);
    return;
  }
  uint stack_channel = 0;
  par.param("st_ch", stack_channel);
  Blob_mt_mapper* mapper = new Blob_mt_mapper(imageStacks[stackName], 1);
  cout << "Starting the blob_mt_mapper" << endl;
  mtMappers.insert(make_pair(imageStacks[stackName], mapper));
  mapper->mapBlobs(stack_channel, minEdge, minPeak);
}

// makes a set of smaller stacks of size w*w (full depth ones) and then
// maps blobs in some number of these concurrently. Image stacks are 
// subsequently deleted, and the blobMappers stored without the blobs,
// and their internal maps. This in order not to run out of memory.. !!

void ImageBuilder::map_blobs(f_parameter& par)
{
  uint max_thread=16;
  uint stack_width = 256;
  uint stack_no = 4;     // the number of stacks to map concurrently.
  uint wi;
  float min_edge, min_peak;
  unsigned int mapper_id = 0;
  vector<Blob_mt_mapper*> mappers;  // later we want to replace this with a smarter object
  QString map_name;
  QString errorMessage;
  QTextStream qts(&errorMessage);
  
  if(!par.param("wi", wi))
    qts << "map_blobs please specify wave index (wi=)\n";
  if(!par.param("edge", min_edge) || !par.param("peak", min_peak))
    qts << "map_blobs please specify both min edge (edge=) and min peak (peak=)\n";
  if(!par.param("id", mapper_id))
    qts << "map_blobs please specify a mapper id (n^2)\n";
  if(!par.param("map_name", map_name) || mapper_collections.count(map_name))
    qts << "map_blobs please specify a mapper name (for the collection, map_name=). Name should be unique\n";
  if(errorMessage.length()){
    warn(errorMessage);
    return;
  }
  // optional background subtraction... 
  uint bg_xm, bg_ym;
  float bg_q;
  bg_xm = bg_ym = 0;
  bg_q = 0.1;  // default
  if(par.param("bgx", bg_xm))
    bg_ym = bg_xm;
  par.param("bgy", bg_ym);
  par.param("bgq", bg_q);

  // optional creation of a blobModel .. 
  int xy_radius = 0;
  int z_radius = 0;
  //  int model_multiplier = 1;
  BlobModel* blobModel = 0;
  par.param("bmx", xy_radius);
  par.param("bmz", z_radius);
  //par.param("bm_mult", model_multiplier);
  if(xy_radius > 0 && z_radius > 0)
    blobModel = new BlobModel(0, 0, 0, 1.0, xy_radius, z_radius);
  // we need to remember blobModel, but first lets see if we crash..
  
  bool use_cmap = true;
  par.param("cmap", use_cmap);
  par.param("w", stack_width);
  par.param("n", stack_no);
  stack_no = stack_no == 0 ? 1 : stack_no;
  stack_no = stack_no > max_thread ? max_thread : stack_no;
  stack_width = (int)stack_width > data->pwidth() ? data->pwidth() : stack_width;
  stack_width = stack_width == 0 ? 256 : stack_width;
  QSemaphore* sem = new QSemaphore(stack_no);
  mapper_collection_semaphores.insert(make_pair(map_name, sem));
  
  for(int x=0; x < data->pwidth(); x += stack_width){
    for(int y=0; y < data->pheight(); y += stack_width){
      /// semaphore used to restrict the number of stacks created at any given time
      /// we use the same semaphore that is used by the mapper in the mapBlobs function, since
      /// this means that we can only create a new image stack when the mapper has finished mapping
      /// and deleted it's internal ImStack. 
      stack_info s_info(wi, x, y, 0, stack_width, stack_width, data->sectionNo());
      Blob_mt_mapper* mapper = new Blob_mt_mapper(s_info, data, mapper_id, true);
      // Need to rewrite to make use of use_cmap ..
      //  Mapper should be able to get it's own data from thingy. and do the correct thing.

      // sem->acquire();
      // ImStack* stack = imageStack(wi, x, y, 0, stack_width, stack_width, data->sectionNo(), use_cmap);
      // if(!stack){
      // 	sem->release();
      // 	continue;
      // }
      // sem->release();
      // Blob_mt_mapper* mapper = new Blob_mt_mapper(stack, mapper_id, true);  // stack will be deleted at end of run()

      if(bg_xm)
	mapper->setBgPar(bg_xm, bg_ym, bg_q);
      if(blobModel)
	mapper->setBlobModel(blobModel);
      cout << x << "," << y << " starting mapBlobs available : " << sem->available() << endl;
      mapper->mapBlobs(0, min_edge, min_peak, sem);
      cout << "\t\tmapBlobs returned available : " << sem->available() << endl; 
      mappers.push_back(mapper); 
    }
  }
  // at which point all mapping should have been done.
  cout << "Created a total of " << mappers.size() << " map objects" << endl;
  if(mapper_collections.count(map_name)){
    for(uint i=0; i < mapper_collections[map_name].size(); ++i)
      delete mapper_collections[map_name][i];
  }
  mapper_collections.insert(make_pair(map_name, mappers));
  if(blobModel){
    // do something reasonable..
    if(blobModels.count(map_name)){
      delete blobModels[map_name];
    }
    blobModels[map_name] = blobModel;
  }
}

void ImageBuilder::trainBlobSetModels(f_parameter& par)
{
  vector<QString> parNames;
  QString blobName;
  int xym, zm;  // model sizes.
  QString error;
  bool use_corrected_ids = false;
  QTextStream qts(&error);
  if(!par.param("blobs", blobName))
    qts << "trainBlobSetModels please specify blob collection name (blobs=..)\n";
  if(!mapper_sets.count(blobName))
    qts << "No blob collection with name : " << blobName << "\n";
  if(!par.param("xym", xym) || !par.param("zm", zm))
    qts << "trainBlobSetModels please specify both model dimensions (xym=.. zm=..)";
  if(!par.param("par", ',', parNames))
    qts << "trainBlobSetModels specify parameter names : (par=par1,par2,par3...)";
  if(error.length()){
    warn(error);
    return;
  }
  par.param("corrected_ids", use_corrected_ids);
  
  mapper_sets[blobName]->resetModelFits();
  mapper_sets[blobName]->trainModels(parNames, xym, zm, use_corrected_ids);
}

void ImageBuilder::map_perimeters(f_parameter& par){
  QString mapName;
  float minValue;
  int min, max;  // square root of the minimum area.. 
  unsigned int z;
  bool use_cmap = true;
  unsigned int wi;
  
  QString error;
  QTextStream qts(&error);
  if(!par.param("map", mapName))
    qts << "Please specify the perimeter map name (map=..)\n";
  //  if(perimeterData.count(mapName) || perimeterWindows.count(mapName))
  //  qts << "Please use a unique map name : " << mapName << " already used\n";
  if(!par.param("min", min))
    qts << "Please specify a minimum size of the perimeter (min=..)\n";
  if(!par.param("max", max))
    qts << "Specify max size of perimeter (max=..)\n";
  if(!par.param("wi", wi))
    qts << "Specify wave index to use : (wi=..)\n";
  if(wi >= channels.size())
    qts << "Wave index specified is too large " << wi << " >= " << channels.size() << "\n";
  if(!par.param("value", minValue))
    qts << "Speicfy min value for perimeters (value=..)\n";
  if(!par.param("z", z))
    qts << "Specify which section to use (z=..)\n";
    
  if(error.length()){
    warn(error);
    return;
  }
  par.param("use_cmap", use_cmap);
  
  if(!imageAnalyser)
    imageAnalyser = new ImageAnalyser(data);
  
  if(!imageAnalyser){
    warn("map_perimeters unable to make an imageAnalyser object");
    return;
  }
  
  float* image = new float[data->pwidth() * data->pheight()];
  if(!data->readToFloat(image, 0, 0, z, data->pwidth(), data->pheight(), 1, wi, use_cmap)){
    warn("map_perimeters unable to obtain image using readToFloat");
    delete []image;
    return;
  }
  
  PerimeterData perData = imageAnalyser->findPerimeters(image, data->pwidth(), data->pheight(), 
							min, max, minValue);
  // PerimeterData, or perSets at least does some sort of reference counting, so I believe I can just
  // do
  perimeterData[mapName] = perData;
  if(!perimeterWindows.count(mapName))
    perimeterWindows[mapName] = new PerimeterWindow(data);
  perimeterWindows[mapName]->setPerimeters(perData.perimeterData, channels[wi], z);
  perimeterWindows[mapName]->resize(500, 600);
  perimeterWindows[mapName]->show();
  perimeterWindows[mapName]->raise();
}

void ImageBuilder::project_perimeters(f_parameter& par)
{
  QString mapName;
  unsigned char r=255;
  unsigned char g=255;
  unsigned char b=255;
  unsigned char alpha = 125;
  bool clear=true;
  
  QString error;
  QTextStream qts(&error);
  if(!par.param("map", mapName))
    qts << "Please specify the perimeter map name (map=..)\n";
  if(!perimeterWindows.count(mapName))
    qts << "Unknown mapName : " << mapName << "\n";
  
  if(error.length()){
    warn(error);
    return;
  }

  par.param("r", r);
  par.param("g", g);
  par.param("b", b);
  par.param("a", alpha);
  par.param("clear", clear);

  if(clear)
    memset((void*)overlayData, 0, sizeof(unsigned char) * 4 * data->pwidth() * data->pheight());
  vector<Perimeter> perimeters = perimeterWindows[mapName]->selectedPerimeters();
  for(unsigned int i=0; i < perimeters.size(); ++i)
    overlayPoints(perimeters[i].perimeterPoints(), r, g, b, alpha);
    
  setBigOverlay(overlayData, 0, 0, data->pwidth(), data->pheight());
}

// maps blob_sets to perimeters specified by some QString
void ImageBuilder::map_blobs_to_perimeters(f_parameter& par)
{
  QString blobName;
  QString mapName;  // the name of the perimeterWindow, and given to the neighborMapper
  int mapperMargin;  // max distance between dots I believe..
  int imageMargin;   // don't use blobs next to the edge, due to background subtraction artefacts
  
  QString error;
  QTextStream qts(&error);
  if(!par.param("blobs", blobName))
    qts << "Specify name of mapper sets (blobs=..)\n";
  if(!mapper_sets.count(blobName))
    qts << "Unknown mapper set: " << blobName << "\n";
  if(!par.param("mapName", mapName))
    qts << "Specify name of perimeter window (mapName=..)\n";
  if(!perimeterWindows.count(mapName))
    qts << "Unknown perimter window : " << mapName << "\n";
  if(!par.param("max", mapperMargin))
    qts << "Specify max point point distance (max=..)\n";
  if(!par.param("margin", imageMargin))
    qts << "Specify suitable image Margin\n";
  if(error.length()){
    warn(error);
    return;
  }
  vector<Perimeter> nuclei = perimeterWindows[mapName]->selectedPerimeters();

  vector<QString> paramNames;
  vector<unsigned int> superIds;
  par.param("filter", ',', paramNames);
  par.param("set_ids", ',', superIds);
  bool use_corrected_ids = false;
  par.param("corrected_ids", use_corrected_ids);

  vector<blob_set> blobs = mapper_sets[blobName]->blobSets(superIds, paramNames, use_corrected_ids);
  vector<threeDPoint> points;
  vector<unsigned int> point_indices; // since we don't include everything..
  points.reserve(blobs.size());
  point_indices.reserve(blobs.size());

  QString NNmapperName = mapName;
  par.param("nn_map_name", NNmapperName);
  
  // select points lying with the margins specified..
  int stack_width, stack_height, stack_depth;
  if(!mapper_sets[blobName]->mapperDimensions(stack_width, stack_height, stack_depth)){
    cerr << "ImageBuilder map_blobs_to_perimeter unable to get stack position" << endl;
    exit(1);
  }
  int w=data->pwidth();
  int h=data->pheight();
  blob* b;
  int x; int y; int z;
  for(unsigned int i=0; i < blobs.size(); ++i){
    b = blobs[i].b(0);
    if(!b)
      continue;
    x = b->peakPos % stack_width;
    y = (b->peakPos % (stack_width * stack_height)) / stack_width;
    z = b->peakPos / (stack_width * stack_height);
    blobs[i].adjust_pos(x, y, z);  // sets the offsets..
    if(x <= imageMargin || y < imageMargin)
      continue;
    if(x >= (w - imageMargin) || y >= (h - imageMargin))
      continue;
    points.push_back(threeDPoint(x, y, z, i));
    point_indices.push_back(i);
  }
  if(!neighborMappers.count(NNmapperName))
    neighborMappers[NNmapperName] = new NNMapper2(mapperMargin);
  neighborMappers[NNmapperName]->setMaxDistance(mapperMargin);
  
  neighborMappers[NNmapperName]->mapPoints(points, point_indices, nuclei);
  vector<int> groupIds = neighborMappers[NNmapperName]->pointNuclearIds();  // pointGroupIds or pointNuclearIds
  // and then use these to draw the points in some way.. 
  if(groupIds.size() != points.size()){
    cerr << "GROUPIDS SIZE IS NOT THE SAME AS POINTS SIZE ? " << endl;
    return;
  }
  vector<QColor> colors = generateColors(255);
  memset((void*)overlayData, 0, sizeof(unsigned char) * 4 * data->pwidth() * data->pheight());
  for(unsigned int i=0; i < point_indices.size(); ++i){
    QColor col = colors[ groupIds[i] % colors.size()];
    blob_set& b = blobs[ point_indices[i] ];
    stack_info pos = b.position();
    for(unsigned int j=0; j < b.size(); ++j)
      project_blob( b.b(j), pos, col );
  }
  setBigOverlay(overlayData, 0, 0, data->pwidth(), data->pheight());
}

void ImageBuilder::draw_blob_model(f_parameter& par)
{
  QString modelName;
  unsigned int res_multiplier = 1.0;
  QString errorMessage;
  QTextStream qts(&errorMessage);
  if(!par.param("name", modelName)){
    qts << "Specify blob model name (name=..)\n" << endl;
  }else{
    if(!blobModels.count(modelName))
      qts << "No model with name " << modelName << "\n";
  }
  if(errorMessage.length()){
    warn(errorMessage);
    return;
  }
  par.param("m", res_multiplier);
  float bias = 0;
  float scale = 1;
  par.param("scale", scale);
  par.param("bias", bias);
  bool clear = true;
  par.param("clear", clear);

  int s_range, z_radius;
  float* model = blobModels[modelName]->model(s_range, z_radius, res_multiplier);
  cout << "called blobModels : " << s_range << "," << z_radius << "," << res_multiplier << endl;
  // the easiest way to draw this map..
  channel_info cinfo(1, color_map(1, 1, 1), 1, bias, scale, false, false);  // pseudo_wave_index, colors, max level, bias, scale, bg_subtract, contrast
  int w = 1 + (2 * z_radius);
  int h = 1 + s_range;
  if(!w || !h)
    return;
  toRGB(model, rgbData, cinfo, w * h,  0, 0, w, h, clear);
  // we can plot a blobModel as well. using a linePlotter. 
  if(!linePlotters.count("BlobModelPlotter"))
    linePlotters["BlobModelPlotter"] = new LinePlotter();
  linePlotters["BlobModelPlotter"]->setData(model, w, h, true, true);
  linePlotters["BlobModelPlotter"]->show();

  delete []model;
  setRGBImage(rgbData, data->pwidth(), data->pheight());  // not the most efficient way of doing it obviously.. 
}

// goes through the specified blobmappers (from mapper_collections) and obtains a set of similarity scores
// plots the distribution of these same ones. Later allow the selective drawing of blobs selected on the
// basis of similarity to the thingy.. 
void ImageBuilder::compare_model(f_parameter& par)
{
  QString mapper_name = "";
  QString errorMessage;
  QTextStream qts(&errorMessage);
  if(!par.param("blobs", mapper_name))
    qts << "No mapper name specified\n";
  if(mapper_name.length() && !mapper_collections.count(mapper_name))
    qts << "No mapper collection named : " << mapper_name << "\n";

  if(errorMessage.length()){
    warn(errorMessage);
    return;
  }
  vector<float> correlations;
  vector<Blob_mt_mapper*>& mappers = mapper_collections[mapper_name];
  for(vector<Blob_mt_mapper*>::iterator it=mappers.begin(); it != mappers.end(); ++it){
    vector<float> corr = (*it)->blob_model_correlations();
    cout << "Inserting correlations into big vector: " << corr.size() << " : " << correlations.size()
	 << endl;
    correlations.insert(correlations.end(), corr.begin(), corr.end());
  }
  if(!correlations.size()){
    warn("compare_model: didn't obtain any blob correlations. Suspect a bug");
    return;
  }
  // default to just plotting the distribution of the scores..
  // the range of values should lie between -1 and +1, but let's not suppose that
  // until it's been checked.

  //  for(uint i=0; i < correlations.size(); ++i)
  //  cout << "  " << correlations[i];
  //cout << endl;

  if(!distPlotters.count("BlobParameters"))
    distPlotters["BlobParameters"] = new DistPlotter(true);  // true = useLimits, don't actually remember what is the meaning
  distPlotters["BlobParameters"]->setSingleData(correlations, true);  // true = resetLimits (make sure to use min to max range.. 
  distPlotters["BlobParameters"]->show();
}

void ImageBuilder::plot_blob_pars(f_parameter& par)
{
  QString mapper_name = "";
  QString x_par = "";
  QString y_par = "";
  QString errorMessage;
  QTextStream qts(&errorMessage);
  if(!par.param("blobs", mapper_name))
    qts << "plot_blob_pars : no mapper (blobs=) name specified\n";
  if(mapper_name.length() && !mapper_collections.count(mapper_name))
    qts << "No mapper collection named : " << mapper_name << "\n";
  if(!par.param("x", x_par))
    qts << "plot_blob_pars : no x parameter specified\n";
  if(!par.param("y", y_par))
    qts << "plot_blob_pars : no y parameter specified\n";
  if(errorMessage.length()){
    warn(errorMessage);
    return;
  }
  vector<float> x_values;
  vector<float> y_values;
  
  vector<Blob_mt_mapper*>& mappers = mapper_collections[mapper_name];
  for(vector<Blob_mt_mapper*>::iterator it=mappers.begin(); it != mappers.end(); ++it){
    vector<blob*> blobs = (*it)->rblobs();
    for(unsigned int i=0; i < blobs.size(); ++i){
      x_values.push_back( getBlobParameter(blobs[i], x_par) );
      y_values.push_back( getBlobParameter(blobs[i], y_par) );
    }
  }
  if(!scatterPlotters.count("BlobPars"))
    scatterPlotters["BlobPars"] = new ScatterPlotter();
  scatterPlotters["BlobPars"]->setData(x_values, y_values);
  scatterPlotters["BlobPars"]->show();
}

void ImageBuilder::plot_blob_set_dist(f_parameter& par)
{
  QString bset_name;
  QString error;
  QTextStream qts(&error);
  if(!par.param("blobs", bset_name))
    qts << "plot_blob_set_pars : no blob set name (blobs=..) specified\n";
  //  if(!mapper_blob_sets.count(bset_name))
  if(!mapper_sets.count(bset_name))
    qts << "plot_blob_set_pars : no blob set with " << bset_name << " known\n";
  //  unsigned int set_id = 0;
  // unsigned int mapper_id = 0;
  
  vector<unsigned int> set_ids;
  vector<unsigned int> mapper_ids;

  if(!par.param("set", ',', set_ids))
    qts << "plot_blob_set_pars no blob set id specfied (set=..)\n";
  if(!par.param("map", ',', mapper_ids))
    qts << "plot_blob_set_pars no mapper id specified (map=..)\n";
  if(error.length()){
    warn(error);
    return;
  }
  set<unsigned int> set_ids_set;
  set_ids_set.insert(set_ids.begin(), set_ids.end());

  QString parName;
  if(!par.param("par", parName))
    qts << "plot_blob_set_pars no parameter name specified (par=..)\n";
  // if(!isPowerOfTwo(mapper_id))
  //   qts << "plot_blob_set_pars mapper id should be an even power of two\n";
  // if(mapper_id > set_id)
  //   qts << "plot_blob_set_pars mapper id should be lower than or equal to set_id\n";
  if(error.length()){
    warn(error);
    return;
  }
  vector<vector<float> > values;
  map<unsigned int, map<unsigned int, vector<float> > > value_sets;  // mapper_id, set_id --> value
  vector<blob_set> blobs = mapper_sets[bset_name]->blobSets();
  //  vector<blob_set>& blobs = mapper_blob_sets[bset_name];
  for(uint i=0; i < blobs.size(); ++i){
    if(set_ids_set.count(blobs[i].id())){
      //    if(blobs[i].id() == set_id){
      for(unsigned int j=0; j < mapper_ids.size(); ++j){
	blob* blob = blobs[i].blob_with_id(mapper_ids[j]);
	if(blob)
	  value_sets[ mapper_ids[j] ][ blobs[i].id() ].push_back( getBlobParameter(blob, parName) );
	//	  values.push_back(getBlobParameter(blob, parName));
      }
    }
  }
  for(map<unsigned int, map<unsigned int, vector<float> > >::iterator it=value_sets.begin();
      it != value_sets.end(); ++it){
    for(map<unsigned int, vector<float> >::iterator iit=(*it).second.begin();
	iit != (*it).second.end(); ++iit){
      values.push_back( (*iit).second );
    }
  }
  if(!distPlotters.count("BlobParameters"))
    distPlotters["BlobParameters"] = new DistPlotter(true);
  DistPlotter* plotter = distPlotters["BlobParameters"];
  plotter->setData(values, true);
  plotter->show();
}

void ImageBuilder::plot_blob_sets(f_parameter& par)
{
  QString errorString;
  QTextStream qts(&errorString);
  if(par.defined("help")){
    warn("plot_blob_sets cells=cell_collection_name cell=cell_no blob_ids=1,2,3");
    return;
  }
  QString cellName;
  unsigned int cell_id;
  set<unsigned int> blob_ids;
  bool use_corrected = false;
  if(!par.param("cells", cellName))
    qts << "Please specify cell collection name cells=..\n";
  if(cellName.length() && !cellCollections.count(cellName))
    qts << "Unknown cellCollection : " << cellName << "\n";
  if(!par.param("cell", cell_id))
    qts << "Please specify cell id cell=..\n";
  if(!par.param("blob", ',', blob_ids))
    qts << "Please specify blob ids to draw blob=1,2,3..\n";
  if(errorString.length()){
    warn(errorString);
    return;
  }
  Cell2 cell;
  if(!cellCollections[cellName]->cell(cell_id, cell)){
    qts << "Unable to obtain cell with id : " << cell_id;
    warn(errorString);
    return;
  }
  vector<blob_set*> blob_sets = cell.blobs(blob_ids, use_corrected);
  ////  if we don't preset the imageStacks, then they'll get created and 
  ////  deleted everytime we create a blob_set_space.
  set<Blob_mt_mapper*> bmappers = set_mappers(blob_sets);

  for(set<Blob_mt_mapper*>::iterator it = bmappers.begin(); it != bmappers.end(); ++it)
    (*it)->setImageStack(true);
  vector<blob_set_space*> blob_spaces;
  for(unsigned int i=0; i < blob_sets.size(); ++i)
    blob_spaces.push_back( new blob_set_space(*(blob_sets[i])) );
  for(set<Blob_mt_mapper*>::iterator it = bmappers.begin(); it != bmappers.end(); ++it)
    (*it)->freeImageStack();

  if(!blob_spaces.size())
    return;
  if(!blobSetPlotter)
    blobSetPlotter = new BlobSetPlotter();
  blobSetPlotter->setBlobSet(cellName, cell_id, blob_spaces);
  blobSetPlotter->show();

}

void ImageBuilder::plot_cell_blob_pars(f_parameter& par)
{
  QString collection_name, xpar, ypar;
  vector<unsigned int> cell_ids;
  unsigned int set_id, map_id;
  QString errorString;
  QTextStream qts(&errorString);
  if(par.defined("help")){
    warn("plot_cell_blob_pars cells=coll_name x=par1 y=par2 ids=cell_ids set=blob_set_id map=mapper_id");
    return;
  }
  if(!par.param("cells", collection_name))
    qts << "specify cell collection name : cells=..\n";
  if(collection_name.length() && !cellCollections.count(collection_name))
    qts << "unknown cell collection : " << collection_name << "\n";
  if(!par.param("x", xpar))
    qts << "specify xparameter name x=..\n";
  if(!par.param("y", ypar))
    qts << "specify yparameter name y=..\n";
  if(!par.param("ids", ',', cell_ids))
    qts << "specify cell ids to plot params from: ids=a,b,c\n";
  if(!par.param("set", set_id))
    qts << "specify the blob set id to inspect: set=..\n";
  if(!par.param("map", map_id))
    qts << "specify the mapper id to inspect: map=..\n";

  if(errorString.length()){
    warn(errorString);
    return;
  }
  CellParCollector collector( cellCollections[collection_name] );
  cell_pars pars = collector.params(cell_ids, set_id, map_id, xpar, ypar);
  
  if(!scatterPlotters.count("BlobPars"))
    scatterPlotters["BlobPars"] = new ScatterPlotter();
  scatterPlotters["BlobPars"]->setData(pars.x, pars.y);
  scatterPlotters["BlobPars"]->show();
}

void ImageBuilder::mergeBlobs(f_parameter& par)
{
  vector<QString> mapperNames;
  if(!par.param("blobs", ',', mapperNames)){
    warn("mergeBlobs No mapperNames specified (blobs=n1,n2,n3)");
    return;
  }
  // this is a bit clumsy, and has been included primarily because 
  // of some segmentation error in my code elsewhere. This is to allow
  // the user to specify offsets to use when merging different blobs
  map<QString, ChannelOffset> offsetMap;
  for(unsigned int i=0; i < mapperNames.size(); ++i)
    offsetMap[mapperNames[i]] = ChannelOffset();  // defalt to 0,0,0
  vector<int> offsets;
  if(par.param("offsets", ',', offsets)){
    if(offsets.size() == mapperNames.size() * 3){
      for(unsigned int i=0; i < offsets.size(); i += 3){
	ChannelOffset co(i, offsets[i], offsets[i+1], offsets[i+2]);
	offsetMap[ mapperNames[i/3] ] = co;
	cout << "inserting into offsetMap key: " << mapperNames[i/3].ascii() << " : "
	     << i << " : " << offsets[i] << "," << offsets[i+1] << "," << offsets[i+2] << endl;
	cout << "   and co : " << co.x() << "," << co.y() << "," << co.z() << endl;
      }
    }else{
      warn("channel offsets must be specified for each channel offsets=x1,y1,z1,x2,y2,etc\n");
    }
  }
  unsigned int radius = 1;
  par.param("r", radius);
  
  vector<vector<Blob_mt_mapper*> > mappers;
  vector<QString> used_mapperNames;
  vector<ChannelOffset> ch_offsets;
  for(unsigned int i=0; i < mapperNames.size(); ++i){
    if(mapper_collections.count(mapperNames[i])){
      mappers.push_back(mapper_collections[mapperNames[i]]);
      ch_offsets.push_back(offsetMap[mapperNames[i]]);
      cout << "Pushing back ch_offsets name : " << mapperNames[i].ascii() << " :: " << offsetMap[mapperNames[i]].x() 
	   << "," << offsetMap[mapperNames[i]].y() << "," << offsetMap[mapperNames[i]].z() << endl;
      used_mapperNames.push_back(mapperNames[i]);  // remove these from mapper_collections if successful.. 
    }else{
      warn("mergeBlobs unknown mapper collection name");
    }
  }
  if(mappers.size() < 2){
    warn("mergeBlobs mapper size is less than two");
    return;
  }
  vector<blob_set> blob_sets;
  // check that the sizes of the mapper collections are the same..
  unsigned int map_no = mappers[0].size();
  if(!map_no){
    warn("mergeBlobs map collections appear to be empty");
    return;
  }
  for(uint i=0; i < map_no; ++i){
    vector<Blob_mt_mapper*> bmt;
    for(uint j=0; j < mappers.size(); ++j){
      if(mappers[j].size() != map_no){
	warn("mergeBlobs individual maps contain different numbers of submaps");
	return;
      }
      bmt.push_back(mappers[j][i]);
    }
    /// Here the blob_mt_mapper blob_sets() funtion is called. We should actually
    /// change that and use a BlobMerger object directly (this is called in the
    /// blob_mt_mapper function. However, at the moment, the Blob_mt_mapper
    /// wrapper function does some checking that's not performed by the
    /// BlobMerger function.
    vector<blob_set> bs = bmt[0]->blob_sets(bmt, ch_offsets, radius);  // this does the actual merging.. 
    blob_sets.insert(blob_sets.end(), bs.begin(), bs.end());
    cout << i <<  "  Added " << bs.size() << " blob_sets, giving a total of : " << blob_sets.size() << endl;
  }
  // let's count the numbers..
  map<unsigned int, int> counts;
  map<unsigned int, std::vector<blob_set> > blob_set_map;
  for(unsigned int i=0; i < blob_sets.size(); ++i){
    if(!counts.count(blob_sets[i].id()))
      counts[blob_sets[i].id()] = 0;
    counts[blob_sets[i].id()]++;
    blob_set_map[ blob_sets[i].id() ].push_back(blob_sets[i]);
  }
  cout << "Class Counts : " << endl;
  for(map<unsigned int, int>::iterator it=counts.begin(); it != counts.end(); ++it)
    cout << (*it).first << " : " << (*it).second << endl;
  
  // lets make a Blob_mt_mapper_collection, and then remove the Blob_mt_mappers from the mapper_collections.
  map<unsigned int, vector<Blob_mt_mapper*> > mapper_map;
  for(unsigned int i=0; i < used_mapperNames.size(); ++i){
    mapper_map[ mapper_collections[used_mapperNames[i]][0]->mapId() ] = mapper_collections[ used_mapperNames[i] ];
    mapper_collections.erase(used_mapperNames[i]);
  }
  Blob_mt_mapper_collection* mapper_set = new Blob_mt_mapper_collection();
  mapper_set->setMappers(blob_set_map, mapper_map);

  if(mapper_sets.count(used_mapperNames[0]))
    delete mapper_sets[ used_mapperNames[0] ];  // Probably leads to crashes as lots of pointers not cleaned up. 
  mapper_sets.insert(make_pair( used_mapperNames[0], mapper_set));
  QString criteriaFileName;
  if(par.param("criteria", criteriaFileName))
    mapper_set->readCriteriaFromFile(criteriaFileName);

}

void ImageBuilder::readBlobCriteria(f_parameter& par)
{
  QString mapName;
  QString error;
  QTextStream qts(&error);
  if(!par.param("map", mapName))
    qts << "readBlobCriter : no map name specified\n";
  if(!mapper_sets.count(mapName))
    qts << "readBlobCriteria : no map with name : " << mapName << "\n";
  QString fileName;
  if(!par.param("criteria", fileName))
    qts << "readBlobCritera : no fileName specified" << endl;
  if(error.length()){
    warn(error);
    return;
  }
  if(!mapper_sets[ mapName ]->readCriteriaFromFile(fileName))
    warn("Blob_mt_mapper unable to read criteria from file");
  
}

/* make_blob_model is problematic, because in larger data sets it is necessary for
 the blob_mt_mapper object to delete the imStack it was working on. Also it doesn't
 know how the image stack was created; and this is not trivial as an image stack can be
 created through an unlimited number of steps. Hence, it is easier to make the blob model
 immediately after the creation of the blobs. The disadvantage is ofcourse that the parameters
 of the blob model have to be specified prior to the blob-modelling. In the future I may find
 some way of reviving the below function, but for the present moment, I'm not sure as to how to do it
*/
// Requires, the name of a mapper_collection, a width and depth of the model..
// void ImageBuilder::make_blob_model(f_parameter& par)
// {
//   QString collection_name;
//   int xy_radius = 0;
//   int z_radius = 0;
//   QString errorMessage;
//   QTextStream qts(&errorMessage);
//   if(!par.param("blobs", collection_name) || !mapper_collections.count(collection_name))
//     qts << "make_blob_model: specify name of blob_collection (list to list objects)\n";
//   if(!par.param("xyr", xy_radius) || xy_radius <= 0)
//     qts << "make_blob_model: specify xy_radius (xyr = w where w > 0)\n";
//   if(!par.param("zr", z_radius) || zy_radius <= 0)
//     qts << "make_blob_model: specfy z_radius (zr=d where d > 0)\n";
//   if(errorMessage.length()){
//     warn(errorMessage);
//     return;
//   }
//   // the 0,0,0, 1.0 don't actually mean anything. 
//   BlobModel* bmodel = new BlobModel(0, 0, 0, 1.0, xy_radius, z_radius);
//   vector<Blob_mt_mapper*>& mappers = mapper_collections[collection_name];
//   // generally, mappers in mapper_collections won't remember the data (i.e. their
//   // associated imStack is deleted. Since this is necessary to make the model,
//   // we'll need to make a new ImStack for the model. Then get the blobs from it, and access
//   // the imStack for the values. This seems a little bit complicated and could probably benefit from
//   // being simplified in some manner or other.
//   for(for uint i=0; i < mappers.size(); ++i){
//     Blob_mt_mapper* map = mappers[i];
//     ImStack* stack = 0; // unfortunately, map doesn't know the wavelength. That is trouble.
// }

void ImageBuilder::project_blob_collections(f_parameter& par)
{
  vector<QString> collNames;
  if(!par.param("blobs", ',', collNames)){
    warn("Please specivy blob collection names (blobs=name1,name2,name,3);\n");
    return;
  }
  float r, g, b;
  r = g = b = 1.0;
  par.param("r", r);
  par.param("g", g);
  par.param("b", b);  
  qnt_colors qntColors( QColor(255 * r, 255 * g, 255 * b) ); // a default one.
  
  QString qntPar;
  vector<QColor> qColors;
  vector<float> qntBreaks;
  if(par.param("qntpar", qntPar) && par.param("col", qColors) && par.param("breaks", ',', qntBreaks))
    qntColors = qnt_colors(qntPar, qColors, qntBreaks);

  cout << "project_blob_collections qntPar, qColors, qntBreaks : " << qntPar.ascii()
       << " : " << qColors.size() << " : " << qntBreaks.size() << endl;
  cout << "qntColors .. " << qntColors.colors.size() << " : " << qntColors.breaks.size() << endl;

  bool clearData = true;
  par.param("clear", clearData);
  if(clearData)
    memset(overlayData, 0, sizeof(unsigned char) * 4 * data->pwidth() * data->pheight());

  unsigned char alpha = 125;
  par.param("alpha", alpha);

  for(uint i=0; i < collNames.size(); ++i){
    if(!mapper_collections.count(collNames[i]))
      continue;
    for(vector<Blob_mt_mapper*>::iterator it=mapper_collections[collNames[i]].begin();
	it != mapper_collections[collNames[i]].end(); ++it){
      //      project_blobs((*it), r, g, b);
      project_blobs((*it), qntColors, alpha);
    }
  }
}

void ImageBuilder::project_blob_sets(f_parameter& par)
{
  QString blob_set_name;
  if(!par.param("blobs", blob_set_name)){
    warn("project_blob_sets : no blob_set specified (blobs=..)");
    return;
  }
  //  if(!mapper_blob_sets.count(blob_set_name)){
  if(!mapper_sets.count(blob_set_name)){
    warn("project_blob_sets : unknown blob_set_name");
    return;
  }
  // some default colors and stuff.. 
  unsigned char alpha = 120;
  par.param("alpha", alpha);
  vector<QColor> colors = generateColors(alpha);
 
  par.param("color", colors);  // uses ; to define.. 

  bool clear = true;
  par.param("clear", clear);
  if(clear)
    memset((void*)overlayData, 0, sizeof(unsigned char) * 4 * data->pwidth() * data->pheight());
  
  for(unsigned int i=0; i < colors.size(); ++i)
    colors[i].setAlpha(alpha);

  // We can also specify a vector of superIds, or a vector of param names to use
  // with an exisiting parameter set .. 
  vector<QString> paramNames;
  vector<unsigned int> superIds;
  par.param("filter", ',', paramNames);
  par.param("set_ids", ',', superIds);
  
  bool use_corrected_ids = false;
  par.param("corrected_ids", use_corrected_ids);
  // the blobSets() functions handles empty vectors to indicate all superIds or no parameters
  vector<blob_set> blobs = mapper_sets[blob_set_name]->blobSets(superIds, paramNames, use_corrected_ids);
  for(unsigned int i=0; i < blobs.size(); ++i){
    stack_info pos = blobs[i].position();
    for(unsigned int j=0; j < blobs[i].size(); ++j)
      project_blob( blobs[i].b(j), pos, colors[ blobs[i].id() % colors.size() ]);
  }
  setBigOverlay(overlayData, 0, 0, data->pwidth(), data->pheight());
}

void ImageBuilder::dilate_blob_sets(f_parameter& par)
{
  QString errorString;
  QTextStream qts(&errorString);
  if(par.defined("help")){
    qts << "dilate_blobs blobs=blob_id xr=x_radius yr=y_radius zr=z_radius [use_visible=false]";
    warn(errorString);
    return;
  }
  QString blob_id;
  QString stack_name;
  int xr, yr, zr;
  if(!par.param("blobs", blob_id))
    qts << "Please specify the id of the blob_sets to use\n";
  if(blob_id.length() && !mapper_sets.count(blob_id))
    qts << "Unknown blob_sets: " << blob_id << "\n";
  if(!par.param("stack", stack_name))
    qts << "Please specify a name for the stack\n";
  if(!par.param("xr", xr) || !par.param("yr", yr) || !par.param("zr", zr))
    qts << "Please specify xr, yr and zr\n";
  if(errorString.length()){
    warn(errorString);
    return;
  }
  bool use_visible = false;
  par.param("use_visible", use_visible);
  // and then work out the size of the stack to be calculated.
  int x = 0;
  int y = 0;
  int z = 0;
  int width = data->pwidth();
  int height = data->pheight();
  int depth = data->sectionNo();
  if(use_visible){
    if(!setVisible(x, y, width, height)){
      warn("Unable to setVisible");
      return;
    }
  }
  // and at this point we can just make a thingy and call the function.
  PointDilater dilater;
  vector<blob_set> blobs = mapper_sets[blob_id]->blobSets();
  ImStack* imStack = dilater.dilate(x, y, z, width, height, depth,
			     blobs, xr, yr, zr);
  // 
  // show the middle layer of it.
  float* layer = imStack->image(0, depth/2);
  channel_info cinfo = imStack->cinfo(0);
  toRGB(layer, rgbData, cinfo, (unsigned long)(width * height),
	x, y, width, height, true, false);
  setRGBImage(rgbData, data->pwidth(), data->pheight());
  if(imageStacks.count(stack_name))
    delete imageStacks[stack_name];
  imageStacks[stack_name] = imStack;
}

// I think ids refers to cell ids rather than
// blob_set_ids
void ImageBuilder::project_blob_ids(f_parameter& par)
{
  QString blobName;  //
  QString mapName;   // refers to the map name, not the blob name
  unsigned char alpha = 120;
  QString errorString;
  QTextStream qts(&errorString);
  if(!par.param("blobs", blobName))
    qts << "Please specify name of mapper sets (blobs=)\n";
  if(!mapper_sets.count(blobName))
    qts << "Name of mapper set (blobs) does not map to a mapper set\n";
  if(!par.param("map", mapName))
    qts << "Name of map does not map to a neighborMapper (map=)\n";
  if(!neighborMappers.count(mapName))
    qts << "Name of mapper set (blobs) does not map to a NNMapper2\n";
  if(errorString.length()){
    warn(errorString);
    return;
  }
  par.param("alpha", alpha);
  vector<QColor> colors = generateColors(alpha);
  if(par.param("col", colors)){
    for(unsigned int i=0; i < colors.size(); ++i)
      colors[i].setAlpha(alpha);
  }
  if(colors.size() < 2)           // we'll run into trouble later on.. 
    colors = generateColors(alpha);

  set<int> selected_ids;
  par.param("ids", ',', selected_ids);

  vector<unsigned int> pointIndices = neighborMappers[mapName]->pointIndices();
  QString id_type;
  par.param("id_type", id_type);
  if(id_type != "nucleus" && id_type != "group")
    id_type = "nucleus";
  vector<int> ids;
  if(id_type == "nucleus"){
    ids = neighborMappers[mapName]->pointNuclearIds();
  }else{
    ids = neighborMappers[mapName]->pointGroupIds();
  }
  if(ids.size() != pointIndices.size()){
    warn("project_blob_ids ids and pointIndices have different sizes\n");
    return;
  }
  vector<blob_set> blobs = mapper_sets[blobName]->blobSets();

  bool clear = true;
  par.param("clear", clear);
  if(clear)
    memset((void*)overlayData, 0, sizeof(unsigned char) * 4 * data->pwidth() * data->pheight());
  for(uint i=0; i < pointIndices.size(); ++i){
    if(pointIndices[i] >= blobs.size())
      continue;
    if(selected_ids.size() && !selected_ids.count(ids[i]))
      continue;
    QColor col = colors[ 1 + ids[i] % (colors.size() - 1) ];
    if(ids[i] < 0)   // if negative set to 0 position
      col = colors[0];
    blob_set& b = blobs[ pointIndices[i] ];
    stack_info pos = b.position();
    for(unsigned int j=0; j < b.size(); ++j)
      project_blob( b.b(j), pos, col );
  }
  setBigOverlay(overlayData, 0, 0, data->pwidth(), data->pheight());
}

void ImageBuilder::make_cell_mask(f_parameter& par)
{
  QString mapName; // NNMapper name
  unsigned char alpha = 80;
  QString error;
  bool clear = true;
  unsigned int max_distance;
  unsigned short border_increment = 10000;
  QTextStream qts(&error);
  if(!par.param("map", mapName))
    qts << "Please specify NNMapper name (map=..)\n";
  if(mapName.length() && !neighborMappers.count(mapName))
    qts << "Unknown NNMapper : " << mapName << "\n";
  if(!par.param("maxd", max_distance))
    qts << "Specify max connection distance (maxd=..)\n";
  if(error.length()){
    warn(error);
    return;
  }
  par.param("alpha", alpha);
  vector<QColor> colors = generateColors(alpha);
  if(par.param("col", colors)){
    for(unsigned int i=0; i < colors.size(); ++i)
      colors[i].setAlpha(alpha);
  }
  par.param("clear", clear);
  if(cellIdMasks.count(mapName))
    delete cellIdMasks[ mapName ];
  
  unsigned short* buffer = new unsigned short[ data->pwidth() * data->pheight() ];
  cellIdMasks[ mapName ] = neighborMappers[ mapName ]->cellMask2D(buffer, 0, 0, 
					 (unsigned int)data->pwidth(), (unsigned int)data->pheight(),
					 max_distance, border_increment);
  overlayCellMask( cellIdMasks[ mapName ]->cellMask, border_increment, 0, 0, data->pwidth(), data->pheight(), colors, clear );
}

void ImageBuilder::make_cells(f_parameter& par)
{
  QString errorString;
  QTextStream qts(&errorString);
  QString maskName;  // contains the mask and outlines
  QString mapName;   // points to the Blot_mt_mapper_collection via mapper_sets
  vector<QString> parNames; // the parameter names to use for selecting good blobs
  // first if help defined, make a help statement and send warning.
  if(par.defined("help")){
    warn("make_cells mask=celloutline_map map=mapper_set par=par1,par2,par3");
    return;
  }
  bool use_corrected = false;
  if(par.defined("corrected_id"))
    use_corrected = true;
  if(!par.param("mask", maskName))
    qts << "Please specify outline mask mask=..\n";
  if(!par.param("map", mapName))
    qts << "Please specify mapper_set (Blob_mt_mapper_collection) map=..\n";
  if(!par.param("par", ',', parNames))
    qts << "Please specify criterion parameter names par=par1,par2,par2..\n";
  if(maskName.length() && !cellIdMasks.count(maskName))
    qts << "Unknown mask : " << maskName << " please use known mask\n";
  if(mapName.length() && !mapper_sets.count(mapName))
    qts << "Unknown map : " << mapName << " please use known map (member of mapper_sets)\n";
  if(errorString.length()){
    warn(errorString);
    return;
  }
  
  vector<Perimeter>& nuclei = cellIdMasks[maskName]->nuclei;
  vector<Perimeter>& cells = cellIdMasks[maskName]->cells;
  if(nuclei.size() != cells.size()){
    warn("make_cells cellIdMasks contains unevenly sized nuclei & cells\n");
    return;
  }
  CellCollection* cellCollection = new CellCollection();
  for(unsigned int i=0; i < nuclei.size(); ++i)
    cellCollection->addCell( cells[i], nuclei[i] );
  vector<blob_set> blobs = mapper_sets[mapName]->blobSets(parNames, use_corrected);
  cellCollection->addBlobs(blobs);
  if(cellCollections.count(maskName))
    delete cellCollections[maskName];
  cellCollections[maskName] = cellCollection;

}

void ImageBuilder::set_cell_blobs(f_parameter& par)
{
  QString errorString;
  QTextStream qts(&errorString);
  QString cells;  // cellCollection name
  QString mapName;   // points to the Blot_mt_mapper_collection via mapper_sets
  vector<QString> parNames; // the parameter names to use for selecting good blobs
  QString criteria_file;
  // first if help defined, make a help statement and send warning.
  if(par.defined("help")){
    warn("set_blobs cells=cell_collection map=mapper_set par=par1,par2,par3 [bursting] [criteria=criteria_file]");
    return;
  }
  bool use_corrected = false;
  if(par.defined("corrected_id"))
    use_corrected = true;
  if(!par.param("cells", cells))
    qts << "Please specify cell collection name cells=...\n";
  if(!par.param("map", mapName))
    qts << "Please specify mapper_set (Blob_mt_mapper_collection) map=..\n";
  if(!par.param("par", ',', parNames))
    qts << "Please specify criterion parameter names par=par1,par2,par2..\n";
  if(cells.length() && !cellCollections.count(cells))
    qts << "Unknown cell collection : " << cells << " please use known collection\n";
  if(mapName.length() && !mapper_sets.count(mapName))
    qts << "Unknown map : " << mapName << " please use known map (member of mapper_sets)\n";
  if(errorString.length()){
    warn(errorString);
    return;
  }
  // optionally change the criteria used before doing anything.
  if(par.param("criteria", criteria_file)){
    if(!mapper_sets[mapName]->readCriteriaFromFile(criteria_file)){
      warn("Unable to set criteria from specified file, aborting");
      return;
    }
  }
  if(!par.defined("bursting")){
    cellCollections[cells]->setBlobs( mapper_sets[mapName]->blobSets(parNames, use_corrected) ); // this clears the blobs.
  }else{
    cellCollections[cells]->setBurstingBlobs( mapper_sets[mapName]->blobSets(parNames, use_corrected) );
  }
}

void ImageBuilder::set_nuclear_signals(f_parameter& par)
{
  if(par.defined("help")){
    warn("set_nuclear_signals cells=cell_collection_name wi=wave_index z=z_position");
    return;
  }
  QString errorString;
  QTextStream qts(&errorString);
  QString cells;
  unsigned int wi=0;
  int z;
  if(!par.param("cells", cells))
    qts << "Please specify cell collection name cells=..\n";
  if(!par.param("wi", wi))
    qts << "Please specify wave index to use wi=..\n";
  if(!par.param("z", z))
    qts << "Please specify the z-position z=..\n";
  if(wi >= channels.size())
    qts << "wi is too big, should be less than: " << channels.size() << "\n";
  if(cells.length() && !cellCollections.count(cells))
    qts << "Unknown cell collection specified\n";

  if(errorString.length()){
    warn(errorString);
    return;
  }
  
  CellCollection* cc = cellCollections[cells];
  unsigned int cell_no = cc->cellNumber();
  
  if(!imageAnalyser)
    imageAnalyser = new ImageAnalyser(data);

  for(unsigned int i=0; i < cell_no; ++i){
    unsigned int p_length=0;
    float pixel_sum = 0;
    float* n_pixels = imageAnalyser->perimeterPixels(cc->nucleusPerimeter(i), z, wi, p_length, pixel_sum);
    // at this point we don't use the pixel_sum, but we do use the pixel_sum
    delete []n_pixels;
    cc->setNuclearSum(i, wi, pixel_sum);
  }
}

void ImageBuilder::draw_cells(f_parameter& par)
{
  QString errorString;
  QTextStream qts(&errorString);
  QString cellName;
  unsigned char alpha = 200;
  if(par.defined("help")){
    warn("draw_cells cell=cell_collection_name");
    return;
  }
  if(!par.param("cells", cellName))
    qts << "Please specify cell collection name cells=..\n";
  if(cellName.length() && !cellCollections.count(cellName))
    qts << "Unknown cellCollection : " << cellName << "\n";
  if(errorString.length()){
    warn(errorString);
    return;
  }
  // if we have the cells, then lets make a Drawer and draw onto the overlayData..
  vector<QColor> colors = generateColors(alpha);
  Drawer drawer(overlayData, 0, 0, data->pwidth(), data->pheight());
  drawer.setBackground(QColor(0, 0, 0, 0));
  for(unsigned int i=0; i < cellCollections[cellName]->cellNumber(); ++i){
    drawer.drawLines( cellCollections[cellName]->cellPerimeter(i).qpoints(),
		      colors[ i % colors.size() ], true);
    drawer.drawLines( cellCollections[cellName]->nucleusPerimeter(i).qpoints(),
		      colors[ i % colors.size() ], true);
    if(par.defined("id")){ // draw the ids
      int x,y;
      QString cell_id;
      cell_id.setNum(i);
      cellCollections[cellName]->nucleusPerimeter(i).centerPos(x, y);
      drawText(cell_id, x, y);
    }
  }
  setBigOverlay(overlayData, 0, 0, data->pwidth(), data->pheight(), GLImage::DRAWING_LAYER);
  // I seem to have problems with having two layers of transparent textures. I can only get
  // one of them to work. Hence for the time being textOverlayData is not used.
  //if(par.defined("id"))
  //  setBigOverlay(textOverlayData, 0, 0, data->pwidth(), data->pheight(), GLImage::TEXT_LAYER);
  image->setViewState(GLImage::VIEW);
}

void ImageBuilder::draw_cell(f_parameter& par)
{
  QString errorString;
  QTextStream qts(&errorString);
  QString cellName;
  unsigned int cell_id;
  set<unsigned int> blob_ids;
  unsigned char alpha = 200;
  bool use_corrected = false;
  if(par.defined("help")){
    warn("draw_cells cells=cell_collection_name cell=cell_no blob_ids=1,2,3..");
    return;
  }
  if(!par.param("cells", cellName))
    qts << "Please specify cell collection name cells=..\n";
  if(cellName.length() && !cellCollections.count(cellName))
    qts << "Unknown cellCollection : " << cellName << "\n";
  if(!par.param("cell", cell_id))
    qts << "Please specify cell id cell=..\n";
  if(!par.param("blob", ',', blob_ids))
    qts << "Please specify blob ids to draw blob=1,2,3..\n";
  if(errorString.length()){
    warn(errorString);
    return;
  }
  par.param("alpha", alpha);
  par.param("use_corrected", use_corrected);
  Cell2 cell;
  if(!cellCollections[cellName]->cell(cell_id, cell)){
    qts << "Unable to obtain cell with id : " << cell_id;
    warn(errorString);
    return;
  }
  Drawer drawer(overlayData, 0, 0, data->pwidth(), data->pheight());
  drawer.setBackground(QColor(0, 0, 0, 0));
  drawer.setPenColor(QColor(255, 255, 255, alpha));
  drawer.drawCell(cell, blob_ids, use_corrected);
  setBigOverlay(overlayData, 0, 0, data->pwidth(), data->pheight());
  image->setViewState(GLImage::VIEW);
}

void ImageBuilder::modifyCellPerimeter(f_parameter& par)
{
  unsigned int cell_id;
  QString mapName;
  QString errorString;
  QTextStream qts(&errorString);
  bool clear = true;
  par.param("clear", clear);
  if(!par.param("cell", cell_id))
    qts << "Specify cell id (cell=..)\n";
  if(!par.param("map", mapName))
    qts << "Specify map name : map=..\n";
  if(mapName.length() && !cellIdMasks.count(mapName))
    qts << "Unknown cell map (cellIdMasks) : " << mapName << "\n";
  if(errorString.length()){
    warn(errorString);
    return;
  }
  cell_id--;
  if(cell_id >= cellIdMasks[mapName]->cells.size()){
    qts << "Cell_id (cell=" << cell_id + 1 << ") is larger than max index: "
	<< cellIdMasks[mapName]->nuclei.size() << "\n";
    warn(errorString);
    return;
  }
  // If we are here, make a MaskMaker with the cell outline as the basis.
  // Set the size and width to 2 x the size of the cell outline values
  Perimeter& per = cellIdMasks[mapName]->cells[cell_id];
  
  /// here use editPerimeter, set the source to mapName, and the cell 
  editPerimeter(per, mapName, cell_id, clear);

  // int x = per.xmin() - (per.xmax() - per.xmin())/2;
  // int y = per.ymin() - (per.ymax() - per.ymin())/2;
  // x = x < 0 ? 0 : x;
  // y = y < 0 ? 0 : y;
  // int w = 2 * (1 + per.xmax() - per.xmin());
  // int h = 2 * (1 + per.ymax() - per.ymin());
  // w = (x + w) < data->pwidth() ? w : data->pwidth() - x;
  // h = (y + h) < data->pwidth() ? h : data->pwidth() - y;
  // if(!w || !h){
  //   warn("width or height is null ?\n");
  //   return;
  // }
  // if(maskMaker)
  //   delete maskMaker;
  // maskMaker = new MaskMaker(QPoint(x, y), w, h);
  // maskMaker->setPerimeter( per.qpoints() );
  
  // // clear the overlay and set the new mask
  // QPoint maskPos;
  // unsigned char* maskImage = maskMaker->maskImage(maskPos, w, h);
  // setBigOverlay(maskImage, maskPos.x(), maskPos.y(), w, h);
  // image->setPosition( maskPos.x() + w/2, maskPos.y() + h/2 );
  // image->setMagnification(1.0);
  // image->setViewState(GLImage::DRAW);


  // qts << "maskPos : " << maskPos.x() << "," << maskPos.y()
  // 	      << "  dims: " << w << "x" << h << "\n";
  // warn(errorString);
}

void ImageBuilder::modifyCells(f_parameter& par)
{
  QString mapName;
  QString errorString;
  QTextStream qts(&errorString);
  bool clear = true;
  par.param("clear", clear);
  if(!par.param("cells", mapName))
    qts << "Specify map name : cells=..\n";
  if(mapName.length() && ( !cellCollections.count(mapName)) )
    //  if(mapName.length() && ( !cellIdMasks.count(mapName) || !cellCollections.count(mapName)) )
    qts << "Unknown cell map (cellIdMasks) or cell collection (cellCollections) : " << mapName << "\n";
  if(errorString.length()){
    warn(errorString);
    return;
  }
  // the following calls haven't been implemented yet, but the thought is like this.
  if(!maskMaker)
    makeMaskMaker();      // this also sets up certain connections.

  maskMaker->setCellSource(mapName, -1);
  cellCollections[mapName]->setCurrentCell(-1);
  modifyNextCell(1);   // 1 is the increment   
  
  // modifyNextCell checks maskMaker for the cell source --> a cellcollection
  // and then checks the cell collection for the current cell number
  // increments that, calls editCell with the appropriate perimeter
  // and then sets up connections between maskMaker and this to make sure that
  // modifyNextCell is called appropriately until all cells have been gone through
}

void ImageBuilder::reassign_blobs_cells(f_parameter& par)
{
  QString cellName;
  QString errorString;
  QTextStream qts(&errorString);
  if(par.defined("help")){
    warn("reassign_blobs cells=cellCollectionName");
    return;
  }
  if(!par.param("cells", cellName))
    qts << "reassign_blobs please specify cell collection name (cells=..)\n";
  if(cellName.length() && !cellCollections.count(cellName))
    qts << "Unknown cell collection name : " << cellName << "\n";
  if(errorString.length()){
    warn(errorString);
    return;
  }
  cellCollections[cellName]->reassignBlobs();
}

void ImageBuilder::export_cell_summary(f_parameter& par)
{
  QString cellName;
  QString errorString;
  QString fileName;
  QTextStream qts(&errorString);
  if(par.defined("help")){
    warn("export_cells cells=cellCollectionName file=out_file_name");
    return;
  }
  if(!par.param("cells", cellName))
    qts << "export_cell_summary please specify cell collection name (cells=..)\n";
  if(cellName.length() && !cellCollections.count(cellName))
    qts << "Unknown cell collection name : " << cellName << "\n";
  if(!par.param("file", fileName))
    qts << "Please specify output file name : file=..\n";
  if(errorString.length()){
    warn(errorString);
    return;
  }
  cellCollections[cellName]->writeTextSummary(fileName);
}

void ImageBuilder::write_cells_borders(f_parameter& par)
{
  QString cellName;
  QString errorString;
  QString fileName;
  QTextStream qts(&errorString);
  if(par.defined("help")){
    warn("write_cells cells=cellCollectionName file=out_file_name");
    return;
  }
  if(!par.param("cells", cellName))
    qts << "export_cell_summary please specify cell collection name (cells=..)\n";
  if(cellName.length() && !cellCollections.count(cellName))
    qts << "Unknown cell collection name : " << cellName << "\n";
  if(!par.param("file", fileName))
    qts << "Please specify output file name : file=..\n";
  if(errorString.length()){
    warn(errorString);
    return;
  }
  if(!cellCollections[cellName]->writeCells(fileName)){
    qts << "Unable to write cell boundaries to " << fileName;
    warn(errorString);
  }
}

void ImageBuilder::read_cells_from_file(f_parameter& par)
{
  QString cellName;
  QString errorString;
  QString fileName;
  QTextStream qts(&errorString);
  if(par.defined("help")){
    warn("read_cells cells=cellCollectionName file=out_file_name");
    return;
  }
  if(!par.param("cells", cellName))
    qts << "read_cells please specify cell collection name (cells=..)\n";
  if(!par.param("file", fileName))
    qts << "Please specify input file name : file=..\n";
  if(errorString.length()){
    warn(errorString);
    return;
  }
  cout << "about to call new CellCollection()" << endl;
  CellCollection* collection = new CellCollection();
  cout << "and made CellCollection" << endl;
  if(!collection->readCells(fileName)){
    cout << "Unable to readCells" << endl;
    qts << "Unable to read cell collection from : " << fileName;
    warn(errorString);
    delete collection;
  }
  if(cellCollections.count(cellName))
    delete cellCollections[cellName];
  cellCollections[cellName] = collection;
}

// add optional parameters later..
void ImageBuilder::list_objects(f_parameter& par)
{
  ostringstream os;
  os << "Stack objects\n";
  for(map<QString, ImStack*>::iterator it=imageStacks.begin(); it != imageStacks.end(); ++it){
    os << (*it).first.toAscii().constData() << "\t" << (*it).second->description() << "\n";
    for(multimap<ImStack*, Blob_mt_mapper*>::iterator mit=mtMappers.lower_bound((*it).second);
	mit != mtMappers.upper_bound((*it).second); ++mit){
      os << "mapper\t" << (*mit).second->description() << "\n";
    }
  }
  for(map<QString, vector<Blob_mt_mapper*> >::iterator it=mapper_collections.begin(); it != mapper_collections.end(); ++it){
    os << (*it).first.toAscii().constData() << ":\t" << (*it).second.size() << " mappers\n";
    if(blobModels.count((*it).first))
      os << "\tmodel specified\n";
  }
  if(linePlotters.size()){
    os << "Line Plotters:\n";
    for(map<QString, LinePlotter*>::iterator it=linePlotters.begin(); it != linePlotters.end(); ++it)
      os << (*it).first.toAscii().constData() << "\n";
  }
  emit displayMessage(os.str().c_str());
}

// report some stuff.. 
void ImageBuilder::report_parameter(f_parameter& par)
{
  QString message;
  QTextStream qts(&message);
  if(par.defined("location")){
    qts << "Position: " << data->xpos() << "," << data->ypos()
	    << "  :" << data->width() << "," << data->height() << "\n";
  }
  warn(message);
}

void ImageBuilder::set_bleach_counts(f_parameter& par)
{
  int xo = 0;
  int yo = 0;
  float radius;
  if(!par.param("r", radius)){
    warn("please specify the radius in multiples of the min radius");
    return;
  }
  par.param("xo", xo);
  par.param("yo", yo);
  data->set_bleach_counts(radius, xo, yo);
}

void ImageBuilder::bleach_count_p(f_parameter& par)
{
  int x, y;
  QString message;
  QTextStream qts(&message);
  if(!par.param("x", x))
    qts << "specify x\n";
  if(!par.param("y", y))
    qts << "specify y";
  if(message.length()){
    warn(message);
    return;
  }
  unsigned int count = data->bleach_count_p(x, y);
  qts << "Bleach count " << x << "," << y << " --> " << count;
  warn(message);
}

void ImageBuilder::bleach_count_map(f_parameter& par)
{
  float max_count;
  float* bleach_map = data->bleachCountsMap_f(max_count, true);
  if(!bleach_map)
    return;
  // max -> 1.0 need to scale to 255
  float scale = 255;
  unsigned char alpha = 125;
  par.param("scale", scale);
  par.param("alpha", alpha);
  unsigned char* counts_overlay = setBigGrayOverlay(bleach_map, 0, 0, data->pwidth(), data->pheight(), alpha, scale);
  // and delete..
  delete []bleach_map;
  delete []counts_overlay;
}

void ImageBuilder::plot_blob_bleach_count(f_parameter& par)
{
  if(par.defined("help")){
    warn("plot_blob_bleach_count blobs=bname");
    return;
  }
  QString error;
  QTextStream qts(&error);
  QString par_name = "peak";
  QString bname;
  if(!par.param("blobs", bname))
    qts << "No blob mapper collection specified\n";
  if(bname.length() && !mapper_collections.count(bname))
    qts << "Unknown mapper collection\n";
  if(error.length()){
    warn(error);
    return;
  }
  par.param("par", par_name);
  vector<float> bleach_counts;
  vector<float> y_values;
  vector<Blob_mt_mapper*>& mappers = mapper_collections[bname];
  
  if(!imageAnalyser)
    imageAnalyser = new ImageAnalyser(data);

  for(vector<Blob_mt_mapper*>::iterator it=mappers.begin(); it != mappers.end(); ++it){
    vector<blob*> blobs = (*it)->rblobs();
    int blob_x, blob_y, blob_z;
    unsigned int mapper_channel = (*it)->channel();
    float signal;
    for(unsigned int i=0; i < blobs.size(); ++i){
      if(!(*it)->blob_peak_pos(blobs[i], blob_x, blob_y, blob_z))
	continue;
      if(par_name == "peak"){
	if(!(imageAnalyser->point(signal, blob_x, blob_y, blob_z, mapper_channel)))
	  continue;
      }else{
	// the below gives background corrected values, though
	signal = getBlobParameter(blobs[i], par_name );
      }
      y_values.push_back( signal );
      bleach_counts.push_back((float) (blob_z + data->bleach_count_p(blob_x, blob_y)) );
    }
  }
  // and do the plot.
  QString plot_identifier = "bleach_blob";
  if(!scatterPlotters.count(plot_identifier))
    scatterPlotters[plot_identifier] = new ScatterPlotter();

  // Begin misplaced code // 
  // for some analysis. This code really shouldn't go here, but, this and some following the below ends up here
  map<float, vector<float> > y_by_x;
  for(uint i=0; i < y_values.size(); ++i)
    y_by_x[ bleach_counts[i] ].push_back( y_values[i] );
  vector<float> bleach_levels;
  vector<float> mean_y_values;
  for(map<float, vector<float> >::iterator it=y_by_x.begin(); it != y_by_x.end(); ++it){
    float mean = 0;
    for(uint i=0; i < (*it).second.size(); ++i)
      mean += (*it).second[i];
    mean /= (float)(*it).second.size();
    bleach_levels.push_back((*it).first);
    mean_y_values.push_back(mean);
  }
  vector<vector<float> > x_l;
  vector<vector<float> > y_l;
  x_l.push_back(bleach_counts);
  x_l.push_back(bleach_levels);
  y_l.push_back(y_values);
  y_l.push_back(mean_y_values);
  ////// END of misplaced code ////////

  scatterPlotters[plot_identifier]->setData(x_l, y_l);
  scatterPlotters[plot_identifier]->show();
}

// plots ratios of signal vs. bleach_count for overlapping
// regions. Note that there may be 0 counts for bleaching.
// which makes the plot a bit problematic. Hence exports the
// numbers as text for analysis with R or similar.
void ImageBuilder::plot_bleaching(f_parameter& par)
{
  QString error;
  QTextStream qts(&error);
  if(par.defined("help")){
      qts << "plot_bleaching wi=channel fname=filename";
      warn(error);
      return;
  }
  uint wi = 0;
  if(!par.param("wi", wi))
    qts << "Please define the channel: wi=..";
  QString filename;
  par.param("fname", filename);

  std::vector<BorderArea*> borders = data->borderAreas();
  if(!borders.size()){
    warn("Unable to obtain border areas");
    return;
  }
  if(wi >= borders[0]->wave_no)
    qts << "The specified channel is too large, max: " << borders[0]->wave_no -1;
  if(error.length()){
    warn(error);
    for(uint i=0; i < borders.size(); ++i)
      delete borders[i];
    return;
  }
  std::vector<float> t_signal;
  std::vector<float> n_signal;
  std::vector<float> t_bleach;
  std::vector<float> n_bleach;
  
  std::vector<float> bleach_ratio;
  std::vector<float> signal_ratio;

  for(uint i=0; i < borders.size(); ++i){
    unsigned int l=borders[i]->width * borders[i]->height;
    cout << "plot bleaching i: " << i << "  and l: " << l << std::endl;
    t_signal.reserve(t_signal.size() + l);
    n_signal.reserve(n_signal.size() + l);
    t_bleach.reserve(t_bleach.size() + l);
    n_bleach.reserve(n_bleach.size() + l);
    
    bleach_ratio.reserve(bleach_ratio.size() + l);
    signal_ratio.reserve(signal_ratio.size() + l);
    for(uint j=0; j < l; ++j){
      t_signal.push_back(borders[i]->t_data[wi][j]);
      n_signal.push_back(borders[i]->n_data[wi][j]);
      t_bleach.push_back(borders[i]->t_bleach_count[j]);
      n_bleach.push_back(borders[i]->n_bleach_count[j]);

      if(borders[i]->n_data[wi][j] > 0 && borders[i]->n_bleach_count[j] > 0){
	bleach_ratio.push_back( borders[i]->t_bleach_count[j] / borders[i]->n_bleach_count[j]);
	signal_ratio.push_back( borders[i]->t_data[wi][j] / borders[i]->n_data[wi][j] );
      }
    }
  }
  std::cout << "imageBuilder plot_bleaching, n_signal size: " << n_signal.size() << std::endl;
  for(uint i=0; i < borders.size(); ++i)
    delete borders[i];
  
  QString plot_identifier = "bleach_ratio";
  if(!scatterPlotters.count(plot_identifier))
    scatterPlotters[plot_identifier] = new ScatterPlotter();
  scatterPlotters[plot_identifier]->setData(bleach_ratio, signal_ratio);
  scatterPlotters[plot_identifier]->show();
  // and export the numbers. Quite a lot of them probably.
  if(filename.length()){
    QFile file(filename);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text)){
      QTextStream out(&file);
      for(uint i=0; i < t_signal.size(); ++i)
	out << t_signal[i] << "\t" << n_signal[i] << "\t" << t_bleach[i] << "\t" << n_bleach[i] << "\n";
      file.close();
    }else{
      warn("Unable to open file for writing");
    }
  }
}

void ImageBuilder::setImageCenter(f_parameter& par)
{
  int x, y;
  if(!par.param("x", x) || !par.param("y", y)){
    warn("Specify both x and y positions (x=.. y=..)\n");
    return;
  }
  image->setPosition(x, y);
}

void ImageBuilder::displayStack(ImStack* stack, int z_pos, bool z_is_local, bool clear){
  if(!stack)
    return;
  
  // strictly speaking z_pos can be negative, as it can be a global position.
  // but if local it must be positive and less than stack->d(). z_pos defaults to -1
  if(z_is_local && z_pos < 0)
    z_pos = stack->d() / 2;   // i.e. default to the middle section of the stack

  // if z_pos is global, then convert to local position and sanity check
  z_pos = z_is_local ? z_pos : (z_pos - stack->zp());
  if(z_pos >= (int)stack->d() || z_pos < 0)
    z_pos = stack->d() / 2;

  // a local clear
  if(clear)
    clearRGBSubRect(rgbData, data->pwidth(), data->pheight(), stack->xp(), stack->yp(),
		    stack->w(), stack->h());
      
  for(uint i=0; i < stack->ch(); ++i){
    channel_info cinfo = stack->cinfo(i);
    toRGB(stack->l_image(i, z_pos), rgbData, cinfo, stack->w() * stack->h(),
	  stack->xp(), stack->yp(), stack->w(), stack->h(), false);
  }
  setRGBImage(rgbData, data->pwidth(), data->pheight());
}

void ImageBuilder::overlayPoints(vector<int> points, unsigned char r, unsigned char g, unsigned char b, unsigned char a){
  unsigned int l = data->pwidth() * data->pheight();
  for(unsigned int i=0; i < points.size(); ++i){
    if(points[i] < 0)
      continue;
    if((unsigned int)points[i] >= l)
      continue;
    overlayData[ 4 * points[i] ] = r;
    overlayData[ 1 + 4 * points[i] ] = g;
    overlayData[ 2 + 4 * points[i] ] = b;
    overlayData[ 3 + 4 * points[i] ] = a;
  }
}

void ImageBuilder::overlayCellMask( unsigned short* mask, unsigned short border_increment, 
				    int xoff, int yoff, unsigned int width, unsigned int height,
				    vector<QColor>& colors, bool clear )
{
  if(clear)
    memset((void*)overlayData, 0, sizeof(unsigned char) * 4 * data->pwidth() * data->pheight());
  for(int y=yoff; y < (yoff + (int)height) && y < data->pheight(); ++y){
    for(int x=xoff; x < (xoff + (int)width) &&  x < data->pwidth(); ++x){
      if(!mask[ (y-yoff)*width + (x-xoff) ])
	continue;
      int n = 4 * (y * data->pwidth() + x);
      QColor color(255, 255, 255, 125);
      if(mask[ (y-yoff)*width + (x-xoff) ] < border_increment){
	color = colors[ (mask[ (y-yoff)*width + (x-xoff) ] - 1) % colors.size() ];
      }
      overlayData[ n ] = color.red();
      overlayData[ n+1 ] = color.green();
      overlayData[ n+2 ] = color.blue();
      overlayData[ n+3 ] = color.alpha();
    }
  }
  setBigOverlay(overlayData, xoff, yoff, width, height);
}

vector<QColor> ImageBuilder::generateColors(unsigned char alpha)
{
  vector<QColor> colors;
  colors.push_back(QColor(0, 0, 255, alpha));
  colors.push_back(QColor(0, 255, 0, alpha));
  colors.push_back(QColor(255, 0, 0, alpha));
  colors.push_back(QColor(0, 255, 255, alpha));
  colors.push_back(QColor(255, 0, 255, alpha));
  colors.push_back(QColor(255, 255, 0, alpha));
  colors.push_back(QColor(255, 255, 255, alpha));
  return(colors);
}

bool ImageBuilder::setVisible(int& x, int& y, int& width, int& height)
{
  image->currentView(x, y, width, height);
  // do some sanity checking..
  if(x >= data->pwidth() || y >= data->pheight())
    return(false);    
  x = x < 0 ? 0 : x;
  y = y < 0 ? 0 : y;
  width = (x + width) > data->pwidth() ? (data->pwidth() - x) : width;
  height = (y + height) > data->pheight() ? (data->pheight() - y) : height;
  return(true);
}

void ImageBuilder::makeMaskMaker()
{
  if(maskMaker)
    delete maskMaker;
  maskMaker = new MaskMaker(QPoint(0, 0), 10, 10);
    // the minimum set of connections required.
  connect(image, SIGNAL(mousePressed(QPoint, Qt::MouseButton)),
	  this, SLOT(beginSegment(QPoint, Qt::MouseButton)) );
  connect(image, SIGNAL(mouseMoved(QPoint, Qt::MouseButton)),
	  this, SLOT(extendSegment(QPoint, Qt::MouseButton)) );
  connect(image, SIGNAL(mouseReleased(QPoint, Qt::MouseButton)),
	  this, SLOT(endSegment(QPoint, Qt::MouseButton)) );

  connect(image, SIGNAL(keyPressed(QKeyEvent*)),
	  maskMaker, SLOT(keyPressed(QKeyEvent*)) );
  connect(maskMaker, SIGNAL(maskChanged()),
	  this, SLOT(maskMakerChanged()) );

  connect(maskMaker, SIGNAL(increment(int)), this, SLOT(modifyNextCell(int)));
  connect(maskMaker, SIGNAL(perimeterModified()), this, SLOT(modifyPerimeter()));
}

bool ImageBuilder::editPerimeter(Perimeter per, QString perSource, int perId, bool clear)
{
  int x = per.xmin() - (per.xmax() - per.xmin())/2;
  int y = per.ymin() - (per.ymax() - per.ymin())/2;
  x = x < 0 ? 0 : x;
  y = y < 0 ? 0 : y;
  int w = 2 * (1 + per.xmax() - per.xmin());
  int h = 2 * (1 + per.ymax() - per.ymin());
  w = (x + w) < data->pwidth() ? w : data->pwidth() - x;
  h = (y + h) < data->pwidth() ? h : data->pwidth() - y;
  if(!w || !h){
    warn("width or height is null ?\n");
    return(false);
  }
  if(!maskMaker)
    makeMaskMaker();

  maskMaker->newMask(QPoint(x, y), w, h);
  maskMaker->setPerimeter( per.qpoints(), perId, perSource );
  
  QString message;
  QTextStream qts(&message);
  if(cellCollections.count(perSource)){
    qts << "Editing " << perId << " of " << cellCollections[perSource]->cellNumber();
  }else{
    qts << "Editing " << perId << " of unknown cell number from : " << perSource;
  }
  warn(message);

  // clear the overlay and set the new mask
  if(clear) resetOverlayData();
  QPoint maskPos;
  unsigned char* maskImage = maskMaker->maskImage(maskPos, w, h);
  setBigOverlay(maskImage, maskPos.x(), maskPos.y(), w, h);
  image->setPosition( maskPos.x() + w/2, maskPos.y() + h/2 );
  //image->setMagnification(1.0);
  image->setViewState(GLImage::DRAW);
  return(true);
}

void ImageBuilder::clearTextLayer()
{
  if(textOverlayData)
    memset((void*)textOverlayData, 0, sizeof(unsigned char) * 4 * data->pwidth() * data->pheight());
  image->setBigOverlay(textOverlayData, 0, 0, data->pwidth(), data->pheight(), GLImage::TEXT_LAYER);
}

// for drawing simple pieces of text onto 
// x and y indicate center of position
//
// IMPORTANT: this function uses overlayData to draw into. It was
// intended to use textOverlayData and to use two separate texture
// layers ontop of the main one. However, that isn't working for some
// for unknown reasons. For now, just use the overlayData.
void ImageBuilder::drawText(QString text, int x, int y)
{
  QPainter p;
  QFont label_font;
  label_font.setPixelSize(20);
  int max_dim = 1024;  // arbitrary
  QImage slate(max_dim, max_dim, QImage::Format_ARGB32);
  p.begin(&slate);
  p.setFont(label_font);
  QRect brect = p.boundingRect(0, 0, max_dim, max_dim, Qt::AlignLeft|Qt::AlignTop, text);
  p.end();
  QImage q_img(brect.size(), QImage::Format_ARGB32);
  q_img.fill(0);
  p.begin(&q_img);
  p.setFont(label_font);
  p.fillRect(0, 0, brect.width(), brect.height(), QBrush(QColor(125, 125, 0, 125)));
  p.setPen(QPen(QColor(255, 255, 255, 255), 1));
  p.setRenderHint(QPainter::TextAntialiasing, false);
  p.translate(0, brect.height());
  p.scale(1.0, -1.0);
  p.drawText(brect, Qt::AlignCenter, text);
  p.end();
  const uchar* bits = q_img.constBits();

  // then simply copy to appropriate region.
  int xp = x - (brect.width() / 2);
  int yp = y - (brect.height() / 2);
  xp = xp < 0 ? 0 : xp;
  yp = yp < 0 ? 0 : yp;
  int cp_w = xp + brect.width() < data->pwidth() ? brect.width() : data->pwidth() - xp;
  int cp_h = yp + brect.height() < data->pheight() ? brect.height() : data->pheight() - yp;
  for(int i=0; i < cp_h; ++i){
    for(int j=0; j < 4 * cp_w; ++j)
    memcpy((void*)(overlayData + 4 * (((yp+i) * data->pwidth()) + xp)), 
    	   (void*)(bits + 4 * (i * brect.width())), sizeof(unsigned char) * 4 * cp_w);
  }
  // and done.
}

// this function may destroy image. image should be set to the return value.
// it would probably be safer to take a pointer or reference to image, but.. 
float* ImageBuilder::modifyImage(int x, int y, int w, int h, float* image, f_parameter& par)
{
  // create a p_parameter, and then run a set of pipe functions in the order specified by
  // the optional pfunctions=func1,func2,func3 etc.
  // we may need to create an additional p_parameter for each fraction.. 
  vector<QString> functions;
  if(!par.param("pfunc", ',', functions))
    return(image);
  if(!functions.size())
    return(image);
  ps_function func = 0;
  for(uint i=0; i < functions.size(); ++i){
    f_parameter fpar = par;  // copies everything..
    fpar.setFunction(functions[i]);
    p_parameter ppar = f_to_p_param(fpar, w, h, x, y, image);
    if(!pipe_slice_functions.count( ppar.function )){
      cerr << "No function defined for : " << ppar.function << endl;
      return(image);
    }
    func = pipe_slice_functions[ ppar.function ];
    if(!(*this.*func)(ppar)){
      cerr << "ImageBuilder::modifyImage pipe function returned false returning" << endl;
      return(image);
    }
    image = ppar.image();
  }
  return(image);
}

ImStack* ImageBuilder::imageStack(f_parameter& par)
{
  uint wi = 0;
  if(!par.param("wi", wi)){
    cerr << "ImageBuilder::imageStack need to specify wave indes (wi)" << endl;
    return(0);
  }
  if(wi >= channels.size()){
    cerr << "ImageBuilder::imageStack wave index too large" << endl;
    return(0);
  }
  bool use_cmap = true;
  par.param("cmap", use_cmap);
  uint x=0; 
  uint y=0;
  uint z=0;
  uint w = data->pwidth();
  uint h = data->pheight();
  uint d = data->sectionNo();
  
  par.param("x", x); par.param("y", y); par.param("z", z);
  par.param("w", w); par.param("h", h); par.param("d", d);
  
  x = (int)x >= data->pwidth() ? 0 : x;
  y = (int)y >= data->pheight() ? 0 : y;
  z = (int)z >= data->sectionNo() ? 0 : z;
  w = (int)(x + w) < data->pwidth() ? w : (data->pwidth() - x);
  h = (int)(y + h) < data->pheight() ? h : (data->pheight() - y);
  d = (int)(z + d) < data->sectionNo() ? d : (data->sectionNo() - z);
  if(!w || !h || !d){
    cerr << "ImageBuilder::imageStack bad dimensions specified" << endl;
    return(0);
  }
  // which should be safe though we might run out of memory.. 
  float* stack = new float[w * h * d];
  memset((void*)stack, 0, sizeof(float) * w * h * d);
  // and then simply ..
  if(!data->readToFloat(stack, x, y, z, w, h, d, wi, use_cmap)){
    delete []stack;
    cerr << "ImageBuilder::imageStack unable to read from panels. " << endl;
    return(0);
  }
  ImStack* imStack = new ImStack(stack, channels[wi], x, y, z, w, h, d);
  return(imStack);
}
 
ImStack* ImageBuilder::imageStack(uint wi, int x, int y, int z, uint w, uint h, uint d, bool use_cmap)
{
  if(!w || !h || !d || wi >= channels.size())
    return(0);
  float* stack = new float[w * h * d];
  memset((void*)stack, 0, sizeof(float) * w * h * d);
  if(!data->readToFloat(stack, x, y, z, w, h, d, wi, use_cmap)){
    delete []stack;
    return(0);
  }
  ImStack* imStack = new ImStack(stack, channels[wi], x, y, z, w, h, d);
  return(imStack);
}
			
p_parameter ImageBuilder::f_to_p_param(f_parameter& fp, int w, int h, int xo, int yo, float* img)
{
  map<string, float> pars;
  bool ok;
  map<QString, QString> fmap = fp.params();
  for(map<QString, QString>::iterator it=fmap.begin(); it != fmap.end(); ++it){
    float f = (*it).second.toFloat(&ok);
    if(ok)
      pars.insert(make_pair( toString((*it).first), f));
  }
  return(p_parameter(toString(fp.function()), img, w, h, xo, yo, pars));
}

string ImageBuilder::toString(QString qstr)
{
  string str(qstr.toAscii().constData());
  return(str);
}

QString ImageBuilder::generateGeneralHelp()
{
  QString helpText;
  QTextStream qts(&helpText);
  qts << "Pipe Slice Functions" << "\n";
  for(map<string, ps_function>::iterator it=pipe_slice_functions.begin(); it != pipe_slice_functions.end(); ++it)
    qts << it->first.c_str() << "\n";
  qts << "\nGeneral Functions" << "\n";
  for(map<QString, g_function>::iterator it=general_functions.begin(); it != general_functions.end(); ++it)
    qts << it->first << "\n";
  return(helpText);
}

// It would be better to have some data structures that specified the
// required and optional parameters (with default values) automatically
// but for that to be useful all the functions should use some sort of
// argument collector that would need to be somewhat universal. Or perhaps, I could
// feed a structure into the parameter class in a sort of validate manner. (Automatically
// setting the non-specified values to their default values. Returning true if ok.
// That might work ok. But the class describing the requirements would need to be carefully
// designed. So let's do it the stupid way first to see what kind of requirements we need.
QString ImageBuilder::generatePipeSliceHelp(QString& fname)
{
  // Stupid if fname == then make string and return..
  if(fname == "slice")
    return("slice z= ch= \n");
  if(fname == "g_blur")
    return("g_blur radius=\n");
  if(fname == "lg_blur")
    return("lg_blur radius=\n");
  if(fname == "bg_sub")
    return("bg_sub bg_xr= bg_yr= bg_pcnt=(0-1)\n");
  if(fname == "ch_sub")
    return("ch_sub ch= al_neg=(bool) mult= z=\n");
  if(fname == "p_mean")
    return("p_mean ch= cb=(col_begin) rb=(row_begin)\n");
  return("Unknown slice command\n");
}

QString ImageBuilder::generateGeneralFunctionHelp(QString& fname)
{
  // stupid things as before..
  if(fname == "set_panel_bias")
    return("set_panel_bias ch= row= col= s= b=\n");
  if(fname == "set_frame_bgpar")
    return("set_frame_bgpar wi= xm= ym= q=(0-1)\n");
  if(fname == "find_center")
    return("find_center ch= rb=0 cb=0\n");
  if(fname == "f_project")
    return("f_project beg= end= wi= w=width h=height x=0 y=0 cmap=true clear=false\n");
  if(fname == "fg_project")
    return("fg_project wi= z=0 x=0 y=0 d=depth w=width h=height r|radius=0 cmap=true clear=false brep=1 fg_stack=null\n");
  if(fname == "add_slice")
    return("add_slice ch|wi= z= x=0 y=0 w=width h=height cmap=true\n");
  if(fname == "blur_stack")
    return("blur_stack stack= wi=0 r|radius=4 out=(new stack name)\n");
  if(fname == "deblur_stack")
    return("deblur_stack stack= wi=0 r|radius=4 out=(new stack name)\n");
  if(fname == "sub_stack")
    return("sub_stack stacks=a,b(,c) ch_a=0 ch_b=0 mult=1.0 allowNeg=false xoff=0 yoff=0 zoff=0 (does a = a-b or c = a-b)\n");
  if(fname == "stack_bgsub")
    return("stack_bgsub stack= xm=12 ym=12 q=0.1 wi=0 out=self\n");
  if(fname == "loop_stack")
    return("loop_stack stack= clear=true n=1 ms=100 zb=0 ze=depth\n");
  if(fname == "loop_xz")
    return("loop_xz stack= clearImage=true ms=100 wi=0 yb=0 ye=stack_height\n");
  if(fname == "stack_xz")
    return("stack_xz stack= yp=half_height wi=0 clear=truen");
  if(fname == "stack_yz")
    return("stack_yz stack= xp=half_width wi=0 clear=true\n");
  if(fname == "stack_project")
    return("stack_project stack= clear=true wi=0 zp=stack_zp d=stack_depth\n");
  if(fname == "set_stack_par")
    return("set_stack_par stack= wi= par=(sandb s= b=)\n");
  return("Unknown general command\n");
}

set<Blob_mt_mapper*> ImageBuilder::set_mappers(vector<blob_set*> bs)
{
  set<Blob_mt_mapper*> bm;
  for(unsigned int i=0; i < bs.size(); ++i){
    for(unsigned int j=0; j < bs[i]->size(); ++j)
      bm.insert(bs[i]->bm(j));
  }
  return(bm);
}

void ImageBuilder::beginSegment(QPoint p, Qt::MouseButton button)
{
  cout << "beginSegment :: " << button << " == " << Qt::NoButton << endl;
  if(button == Qt::NoButton)
    return;
  if(!maskMaker)
    return;
  maskMaker->startSegment(p);
}

void ImageBuilder::extendSegment(QPoint p, Qt::MouseButton button)
{
  if(!button)
    return;
  if(!maskMaker)
    return;
  maskMaker->addSegmentPoint(p);
  QPoint pos;
  int w, h;
  w = h = 0;
  unsigned char* mask = maskMaker->maskImage(pos, w, h);
  if(w && h)
    setBigOverlay(mask, pos.x(), pos.y(), w, h);
}

void ImageBuilder::endSegment(QPoint p, Qt::MouseButton button)
{
  if(!button)
    return;
  if(!maskMaker)
    return;
  maskMaker->endSegment(p);
  QPoint pos;
  int w, h;
  w = h = 0;
  unsigned char* mask = maskMaker->maskImage(pos, w, h);
  if(w && h)
    setBigOverlay(mask, pos.x(), pos.y(), w, h);
}

void ImageBuilder::maskMakerChanged()
{
  if(!maskMaker)
    return;
  QPoint pos;
  int w, h;
  w = h = 0;
  unsigned char* mask = maskMaker->maskImage(pos, w, h);
  if(w && h)
    setBigOverlay(mask, pos.x(), pos.y(), w, h);
}

// this doesn not accept the current modification
// only goes to the next one.
void ImageBuilder::modifyNextCell(int increment)
{
  if(!maskMaker){
    warn("modifyNextCell no maskMaker present\n");
    return;
  }
  QString cellSource = maskMaker->per_source();
  if(!cellCollections.count(cellSource)){
    warn("modifyNextCell: unknown cellCollection");
    return;
  }
  int nextCell = cellCollections[cellSource]->currentCell() + increment;
  if(nextCell >= 0 && nextCell < (int)cellCollections[cellSource]->cellNumber()){
    cellCollections[cellSource]->setCurrentCell(nextCell);
    editPerimeter(cellCollections[cellSource]->cellPerimeter((unsigned int)nextCell),
		  cellSource, nextCell, true);
  }
  
}

void ImageBuilder::modifyPerimeter()
{
  if(!maskMaker)
    return;
  // find the cell source and the id and call modify perimeter
  // on the cell collection.
  // check that this is the current cell perimeter in the cell collection
  QString cellSource;
  int cell_id;
  vector<QPoint> points = maskMaker->getPerimeter(cell_id, cellSource);
  Perimeter per(points, data->pwidth(), data->pheight());
  if(!cellCollections.count(cellSource)){
    warn("modifyPerimeter unknown cellCollection");
    return;
  }
  if(cell_id != cellCollections[cellSource]->currentCell()){
    warn("modifyPerimeter cell ids are mismatched, giving up");
    return;
  }
  if(!cellCollections[cellSource]->modifyCellPerimeter(cell_id, per)){
    warn("Failed to modify perimeter");
  }
  // no need to do much else I believe.. 
}

// // stupid ugly hack.. 
// float ImageBuilder::getBlobParameter(blob* b, QString parname)
// {
//   if(parname == "volume")
//     return((float)b->points.size());
//   if(parname == "sum")
//     return(b->sum);
//   if(parname == "max")
//     return(b->max);
//   if(parname == "min")
//     return(b->min);
//   if(parname == "aux")
//     return(b->aux1);
//   if(parname == "mean")
//     return( b->sum / (float)b->points.size());
//   return(0);
// }
