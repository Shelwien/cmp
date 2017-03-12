
struct color {
  uint fg,bk;
};

color palette[256] = {
  { 0xFFFFFF, 0x000000 },
  { 0xFFFF55, 0x000000 }, //  { 0xFFFFFF, 0x0000FF },
  { 0x00AA00, 0x000000 },
  { 0xFFFFFF, 0x0000AA },

  { 0x00FFFF, 0x000080 },
  { 0xFFFFFF, 0x000080 },
};

enum pal_ {
  pal_Sep=0,
  pal_Hex,
  pal_Addr,
  pal_Diff,

  pal_Help1,
  pal_Help2,

  pal_MAX
};
