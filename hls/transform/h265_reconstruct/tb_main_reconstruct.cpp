#include "global.h"
#include <iostream>
#include <mc_scverify.h>

using namespace std;

template < int N > class Input {
public:
  Input() { reset(); }

  void operator<<(const ac_int< N > &rhs) {
    src[w_id].set_slc(slc * N, rhs);

    slc++;
    if (!slc)
      w_id++;
  }

  ac_int< N * 32, false > get() { return src[r_id++]; }

  bool empty() { return r_id == w_id; }
  void reset() { slc = r_id = w_id = 0; }

private:
  ac_int< 5, false > slc;
  ac_int< 6, false > r_id;
  ac_int< 6, false > w_id;
  ac_int< N * 32, false > src[64];
};

template < int N > class Output {
public:
  Output() { reset(); }

  void operator<<(const ac_int< N * 32, false > &rhs) { src[w_id++] = rhs; }

  ac_int< N > get() {
    ac_int< N > s = src[r_id].template slc< N >(N * slc++);
    if (!slc)
      r_id++;

    return s;
  }

  bool empty() { return r_id == w_id; }
  void reset() { slc = r_id = w_id = 0; }

private:
  ac_int< 5, false > slc;
  ac_int< 6, false > r_id;
  ac_int< 6, false > w_id;
  ac_int< N * 32, false > src[64];
};

void main_reconstruct(resid_port_t &resid_in, pixel_port_t &pred_in, pixel_port_t &ref_in, pixel_port_t &rec_out);

CCS_MAIN(int argc, char *argv[]) {
  static resid_port_t resid_in;
  static pixel_port_t ref_in;
  static pixel_port_t pred_in;
  static pixel_port_t rec_out;

  bool simulation_successful = 1;

  pixel_t ref[32 * 32];
  pixel_t pred[32 * 32];
  int_16 resid[32 * 32];

  pixel_t rec_ref[32 * 32];
  pixel_t rec_duv[32 * 32];

  Input< 8 > ref_i;
  Input< 8 > pred_i;
  Input< 16 > resid_i;
  Output< 8 > rec_o;

  bool print = 0;

  for (int i = 0; i < 64; i++) {
    srand(i);

    uint_4 depth = 0;
    for (depth = 0; depth < 4; ++depth) {
      uint_4 color = 0;
      for (color = 0; color < 3; ++color) {

        conf_t duv_conf;
        duv_conf.depth = depth;
        duv_conf.color = color;

        two_bit size = duv_conf.size();
        uint_6 width = duv_conf.width();
        cout << "Depth: " << depth << " Color: " << color << " Width: " << width << endl;

        // Generate input
        for (uint_6 y = 0; y < 32; ++y) {
          for (uint_6 x = 0; x < 32; ++x) {
            ref[y * 32 + x] = pixel_t(rand());
            pred[y * 32 + x] = pixel_t(rand());
            resid[y * 32 + x] = int_16(rand());
          }
        }

        unsigned ref_ssd_1st = 0;
        unsigned ref_ssd_2nd = 0;

        if (size != 3) {
          for (int y = 0; y < width; ++y) {
            for (int x = 0; x < width; ++x) {
              ref_i << ref[y * width + x];
              pred_i << pred[y * width + x];
              resid_i << resid[y * width + x];

              int_16 val = resid[y * width + x] + pred[y * width + x];
              rec_ref[y * width + x] = pixel_t(CLIP((int_16)0, (int_16)255, val));

              ref_ssd_1st += (ref[y * width + x] - rec_ref[y * width + x]) * (ref[y * width + x] - rec_ref[y * width + x]);
            }
          }
        } else {
          // Luma 4x4 with TR Skip or Chroma 4x4
          for (int x = 0; x < 32; ++x) {
            ref_i << ref[x];
            pred_i << pred[x];
            resid_i << resid[x];

            if (x < 16) {
              int_16 val = resid[x] + pred[x];

              rec_ref[x] = pixel_t(CLIP((int_16)0, (int_16)255, val));
              ref_ssd_1st += (ref[x] - rec_ref[x]) * (ref[x] - rec_ref[x]);
            } else {
              if (!color) { // TR Skip
                int_16 val = resid[x] + pred[x % 16];

                rec_ref[x] = pixel_t(CLIP((int_16)0, (int_16)255, val));
                ref_ssd_2nd += (ref[x % 16] - rec_ref[x]) * (ref[x % 16] - rec_ref[x]);
              } else {
                int_16 val = resid[x] + pred[x];

                rec_ref[x] = pixel_t(CLIP((int_16)0, (int_16)255, val));
                ref_ssd_2nd += (ref[x] - rec_ref[x]) * (ref[x] - rec_ref[x]);
              }
            }
          }
        }

        if (print) {
          std::cout << "\nInput Resid: \n";
          for (int i = 0; i < width * width; ++i) {
            std::cout << resid[i] << " ";

            if (i % width == width - 1)
              std::cout << std::endl;
          }

          std::cout << "\nInput Pred: \n";
          for (int i = 0; i < width * width; ++i) {
            std::cout << pred[i] << " ";

            if (i % width == width - 1)
              std::cout << std::endl;
          }

          std::cout << "\nInput Ref: \n";
          for (int i = 0; i < width * width; ++i) {
            std::cout << ref[i] << " ";

            if (i % width == width - 1)
              std::cout << std::endl;
          }
        }

        resid_in.write(duv_conf.toInt());

        while (!ref_i.empty())
          ref_in.write(ref_i.get());
        while (!pred_i.empty())
          pred_in.write(pred_i.get());
        while (!resid_i.empty())
          resid_in.write(resid_i.get());

        CCS_DESIGN(main_reconstruct)
        (resid_in, pred_in, ref_in, rec_out);

        rec_out.read(); // Flush config
        while (rec_out.available(2)) {
          // Read all but last
          rec_o << rec_out.read();
        }

        pixel_slice_t output = rec_out.read();
        unsigned duv_ssd = output.slc< 32 >(0);
        unsigned duv_1st = output.slc< 32 >(32);
        unsigned duv_2nd = output.slc< 32 >(64);

        if (size != 3) {
          for (int y = 0; y < width; ++y) {
            for (int x = 0; x < width; ++x) {
              rec_duv[y * width + x] = rec_o.get();

              if (rec_duv[y * width + x] != rec_ref[y * width + x]) {
                simulation_successful = 0;
                std::cout << "ERROR: Size " << size << " Rec" << std::endl;
              }
            }
          }

          if (duv_ssd != ref_ssd_1st) {
            simulation_successful = 0;
            std::cout << "ERROR: Size " << size << " SSD" << std::endl;
          }

          if (print)
            std::cout << "SSD Ref: " << ref_ssd_1st << " Duv: " << duv_ssd << std::endl;
        } else {
          for (int x = 0; x < 32; ++x) {
            rec_duv[x] = rec_o.get();

            if (rec_duv[x] != rec_ref[x]) {
              simulation_successful = 0;
              std::cout << "ERROR: Size " << size << " Rec" << std::endl;
            }
          }

          if (duv_1st != ref_ssd_1st) {
            simulation_successful = 0;
            std::cout << "ERROR : duv_1st SSD" << std::endl;
          }

          if (duv_2nd != ref_ssd_2nd) {
            simulation_successful = 0;
            std::cout << "ERROR : duv_2nd SSD" << std::endl;
          }

          if (print) {
            std::cout << "SSD Ref 1st: " << ref_ssd_1st << " Duv: " << duv_1st << std::endl;
            std::cout << "SSD Ref 2nd: " << ref_ssd_2nd << " Duv: " << duv_2nd << std::endl;
          }
        }

        if (!rec_o.empty()) {
          simulation_successful = 0;
          std::cout << "TB ERROR : Output buffer not empty" << std::endl;
        }

        if (!simulation_successful)
          break;
      }
      if (!simulation_successful)
        break;
    }
    if (!simulation_successful)
      break;
  }

  if (simulation_successful == 1) {
    cout << "## SIMULATION PASSED ##" << endl;
  } else {
    cout << "## SIMULATION FAILED ##" << endl;
  }

  CCS_RETURN(0);
}
