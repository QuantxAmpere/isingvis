#include "my_headers.h"

// fps are not **really** used because gnuplot isn't fast enough at replotting
// will probably switch to a custom way of rendering later
#define FPS (60)

// #define DEFAULT_N (42)
// #define DEFAULT_M (42)
#define DEFAULT_N (256 + 128)
#define DEFAULT_M (256)

// we ramp up and back down

//
//
//
// quick animation
// #define RAMP_DURATION (20.0)
// #define LOW_TEMP (0.001)
// #define HIGH_TEMP (15)
//
//
//
//
// slower, more interesting animation, that covers a little less highs, thus
// spending even more time in the lows
#define RAMP_DURATION (60.0)
#define LOW_TEMP (0.001)
#define HIGH_TEMP (6)
//
//
//
//
//

struct State {
  int N;
  int M;
  bool *spins;
  double temperature;
  double mag;
  int frame;
  bool liveAnimation;
  bool averages;
};

void print(struct State *);
void prepare_for_plotting(struct State *);
void simulationStep(struct State *);
// double energy(struct State *);

int main(int argc, char **argv) {
  // some calculations to determine how the animation should look like
  int frames = FPS * RAMP_DURATION;
  // we divide by 2 because we will ramp up and back down
  double dTdt = (HIGH_TEMP - LOW_TEMP) / (double)frames;

  struct State state;
  state.N = DEFAULT_N;
  state.M = DEFAULT_M;
  state.spins = (bool *)malloc(sizeof(bool) * state.N * state.M);
  for (int i = 0; i < state.N * state.M; i++) {
    state.spins[i] = 1;
  }
  state.frame = 0;
  state.temperature = LOW_TEMP;

  state.liveAnimation = false;
  state.averages = false;

  // mode is 2 bit number: liveanimation, averages
  if (argc > 1) {
    int mode = atoi(argv[1]);
    state.liveAnimation = mode & 2;
    state.averages = mode & 1;
  }

  if (!state.averages) {
    prepare_for_plotting(&state);
    srand(time(NULL));
    // this is some black magic to make gnuplot work nicely with my tiling wm
    print(&state);
    usleep(2000 * 1000);
    print(&state);
    usleep(2000 * 1000);
  }

  // main simulation loop
  // ramp up
  for (int i = 0; i < frames; i++) {
    simulationStep(&state);
    state.temperature += dTdt;
    print(&state);
  }
  // ramp down
  for (int i = 0; i < frames; i++) {
    simulationStep(&state);
    state.temperature -= dTdt;
    print(&state);
  }

  return 0;
}

void simulationStep(struct State *state) {
  state->frame += 1;
  state->mag = 0;
  for (int k = 0; k < 2; k++) {
    for (int i = 0; i < state->N * state->M; i++) {
      // random pos
      int pos = rand() % (state->N * state->M);
      state->spins[pos] = !state->spins[pos];
      // rejection / acceptance process
      // sum the neighbor spins
      int sum = 0;
      sum +=
          state->spins[(pos - 1 + state->M * state->N) % (state->M * state->N)]
              ? 1
              : -1;
      sum += state->spins[(pos + 1) % (state->M * state->N)] ? 1 : -1;
      sum += state->spins[(pos - state->M + state->M * state->N) %
                          (state->M * state->N)]
                 ? 1
                 : -1;
      sum += state->spins[(pos + state->M) % (state->M * state->N)] ? 1 : -1;

      double deltaE = -2 * sum * (state->spins[pos] ? 1 : -1);

      if (deltaE < 0 ||
          ((double)rand() / RAND_MAX) < exp(-deltaE / state->temperature)) {
      } else {
        state->spins[pos] = !state->spins[pos];
      }
      // if we're on the second run, we calculate the magnetization
      if (state->averages && k > 0) {
        double mag = 0;
        for (int j = 0; j < state->N * state->M; j++) {
          mag += state->spins[j] ? 1 : -1;
        }
        state->mag += mag / (double)(state->N * state->M);
      }
    }
  }
  if (state->averages)
    state->mag /= (double)(state->N * state->M);
}

// double energy(struct State *state) {
//   double E = 0;
//   // for every spin iterate over its neighbours (up, down, left, right)
//   // periodic boundary conditions
//   // if the spins are identical, add -1 to the energy
//   // if the spins are different, add +1 to the energy
//   for (int i = 0; i < state->N; i++) {
//     for (int j = 0; j < state->M; j++) {
//       for (int k = -1; k <= 1; k += 2) {
//         for (int l = -1; l <= 1; l += 2) {
//           int i2 = (i + k + state->N) % state->N;
//           int j2 = (j + l + state->M) % state->M;
//           if (state->spins[i * state->M + j] ==
//               state->spins[i2 * state->M + j2]) {
//             E -= 1;
//           } else {
//             E += 1;
//           }
//         }
//       }
//     }
//   }
//   return E;
// }

void print(struct State *state) {
  if (state->averages) {
    FILE *magfile = fopen("mag", "a");
    fprintf(magfile, "%f %f\n", state->temperature, state->mag);
    fflush(magfile);
    fclose(magfile);
  } else {
    if (!state->liveAnimation) {
      printf("set output 'frames/frame%09d.png'\n", state->frame);
    }
    printf("plot '-' matrix with image title '%f'\n", state->temperature);

    for (int i = 0; i < state->N; i++) {
      for (int j = 0; j < state->M; j++) {
        printf("%d ", state->spins[i * state->M + j] * 2 - 1);
      }
      printf("\n");
    }
    printf("e\n");

    fflush(stdout);

    if (state->liveAnimation) {
      if (state->N * state->M < 48 * 48) {
        usleep(1000 * 1000 / FPS);
      }
    }
  }
}

void prepare_for_plotting(struct State *state) {
  if (!state->liveAnimation) {
    printf("set terminal png size 1024,1024\n");
  }
  printf("set palette defined (-1 'black', 1 'white')\n");
  printf("set cbrange [-1:1]\n");
}
