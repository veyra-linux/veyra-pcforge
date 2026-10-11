/* VEYRA Pcforge — симулятор сборки компьютера и работы на заказ. Чистый C, без зависимостей. */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define C(n) "\033[38;5;" #n "m"
#define CLR "\033[2J\033[H"
#define RST "\033[0m"
#define BLD "\033[1m"
#define ACC C(45)
#define DIM C(244)
#define TXT C(252)
#define GOOD C(78)
#define WARN C(221)
#define BAD C(203)
#define GOLD C(220)
#define BC C(61)
#define W 74
#define MAGIC 0x50434633
#define VERSION "v1.1"

enum { K_CPU, K_MB, K_RAM, K_GPU, K_PSU, K_COOL, K_STO, K_CASE, K_UPS, NK };
static const char *KN[NK] = {"Процессоры", "Материнские платы", "Оперативная память", "Видеокарты", "Блоки питания",
                             "Кулеры", "Накопители", "Корпуса", "ИБП (защита питания)"};

typedef struct { const char *name; int kind, price, a, b, c; } Part;

/* CPU: a=сокет b=перф c=TDP | MB: a=сокет b=тип ОЗУ c=макс ГБ | RAM: a=тип b=ГБ
   GPU: a=перф b=видеопамять c=TDP | PSU: a=Вт | COOL: a=Вт охлаждения
   STO: a=ГБ b=скорость c=1 если SSD | CASE: a=бонус охлаждения Вт | UPS: a=защита от сбоев % */
static const Part CAT[] = {
    {"Kora C1 (старый)", K_CPU, 30, 0, 10, 35},
    {"Kora C2", K_CPU, 70, 0, 22, 55},
    {"Kora C3", K_CPU, 120, 0, 35, 65},
    {"Kora C5", K_CPU, 220, 0, 60, 95},
    {"Kora C6", K_CPU, 300, 1, 75, 95},
    {"Kora C7", K_CPU, 420, 1, 100, 105},
    {"Kora C9", K_CPU, 700, 1, 150, 125},
    {"Kora X12", K_CPU, 950, 2, 170, 150},
    {"Kora X24 (серверный)", K_CPU, 1500, 2, 230, 180},
    {"Kora X64 (монстр)", K_CPU, 3500, 2, 400, 280},

    {"Plata B1", K_MB, 60, 0, 4, 16},
    {"Plata B3", K_MB, 110, 0, 4, 32},
    {"Plata B5", K_MB, 150, 0, 4, 64},
    {"Plata M3", K_MB, 150, 1, 4, 32},
    {"Plata M5", K_MB, 200, 1, 4, 64},
    {"Plata M7", K_MB, 350, 1, 5, 128},
    {"Plata M9", K_MB, 560, 1, 5, 256},
    {"Plata S5", K_MB, 420, 2, 4, 64},
    {"Plata S7 (сервер)", K_MB, 600, 2, 4, 128},
    {"Plata S9 (сервер)", K_MB, 800, 2, 5, 256},

    {"DDR4 4 ГБ", K_RAM, 25, 4, 4, 0},
    {"DDR4 8 ГБ", K_RAM, 40, 4, 8, 0},
    {"DDR4 16 ГБ", K_RAM, 70, 4, 16, 0},
    {"DDR4 32 ГБ", K_RAM, 130, 4, 32, 0},
    {"DDR4 64 ГБ", K_RAM, 260, 4, 64, 0},
    {"DDR4 128 ГБ", K_RAM, 520, 4, 128, 0},
    {"DDR5 16 ГБ", K_RAM, 110, 5, 16, 0},
    {"DDR5 32 ГБ", K_RAM, 200, 5, 32, 0},
    {"DDR5 64 ГБ", K_RAM, 380, 5, 64, 0},
    {"DDR5 128 ГБ", K_RAM, 800, 5, 128, 0},
    {"DDR5 256 ГБ", K_RAM, 1600, 5, 256, 0},

    {"Pixel 1050", K_GPU, 130, 25, 4, 75},
    {"Pixel 1650", K_GPU, 190, 40, 4, 75},
    {"Pixel 2060", K_GPU, 330, 60, 8, 160},
    {"Pixel 2070", K_GPU, 480, 85, 8, 175},
    {"Pixel 3080", K_GPU, 750, 150, 12, 250},
    {"Pixel 3090", K_GPU, 1300, 200, 16, 320},
    {"Pixel 4090", K_GPU, 1900, 260, 24, 400},
    {"Compute A100 (дата-центр)", K_GPU, 4500, 380, 40, 300},
    {"Compute H200 (топ)", K_GPU, 9000, 520, 80, 450},

    {"БП 300 Вт", K_PSU, 30, 300, 0, 0},
    {"БП 500 Вт", K_PSU, 60, 500, 0, 0},
    {"БП 650 Вт", K_PSU, 80, 650, 0, 0},
    {"БП 750 Вт", K_PSU, 100, 750, 0, 0},
    {"БП 850 Вт", K_PSU, 130, 850, 0, 0},
    {"БП 1000 Вт", K_PSU, 170, 1000, 0, 0},
    {"БП 1600 Вт", K_PSU, 350, 1600, 0, 0},
    {"БП 2000 Вт", K_PSU, 600, 2000, 0, 0},

    {"Боксовый кулер", K_COOL, 10, 65, 0, 0},
    {"Низкопрофильный кулер", K_COOL, 30, 90, 0, 0},
    {"Башенный кулер", K_COOL, 50, 120, 0, 0},
    {"Большая башня", K_COOL, 90, 180, 0, 0},
    {"Двойная башня", K_COOL, 120, 220, 0, 0},
    {"Жидкостное СО (AIO)", K_COOL, 180, 300, 0, 0},
    {"Кастомная СЖО", K_COOL, 400, 450, 0, 0},

    {"HDD 500 ГБ", K_STO, 25, 500, 10, 0},
    {"HDD 2 ТБ", K_STO, 60, 2000, 15, 0},
    {"HDD 4 ТБ", K_STO, 100, 4000, 16, 0},
    {"HDD 8 ТБ", K_STO, 160, 8000, 18, 0},
    {"HDD 16 ТБ", K_STO, 300, 16000, 20, 0},
    {"SSD SATA 500 ГБ", K_STO, 50, 500, 50, 1},
    {"SSD NVMe 500 ГБ", K_STO, 70, 500, 110, 1},
    {"SSD NVMe 1 ТБ", K_STO, 100, 1000, 120, 1},
    {"SSD NVMe 2 ТБ", K_STO, 180, 2000, 200, 1},
    {"SSD NVMe 4 ТБ", K_STO, 380, 4000, 300, 1},
    {"SSD NVMe 8 ТБ", K_STO, 800, 8000, 350, 1},
    {"SSD Gen5 2 ТБ", K_STO, 320, 2000, 450, 1},

    {"Старый корпус", K_CASE, 20, 0, 0, 0},
    {"Корпус с сеткой", K_CASE, 70, 20, 0, 0},
    {"Продуваемый корпус", K_CASE, 140, 45, 0, 0},
    {"Серверная стойка 4U", K_CASE, 380, 90, 0, 0},

    {"ИБП 600 ВА", K_UPS, 80, 40, 0, 0},
    {"ИБП 1500 ВА", K_UPS, 220, 70, 0, 0},
    {"Онлайн-ИБП", K_UPS, 600, 95, 0, 0},
};
#define NCAT ((int)(sizeof CAT / sizeof CAT[0]))

typedef struct { const char *name; const char *items[8]; int n; } Bundle;
static const Bundle BU[] = {
    {"Офисный старт", {"Kora C3", "Plata B3", "DDR4 16 ГБ", "БП 500 Вт", "Боксовый кулер"}, 5},
    {"Рабочая станция", {"Kora C7", "Plata M5", "DDR4 32 ГБ", "Pixel 2060", "БП 750 Вт", "Башенный кулер"}, 6},
    {"Рендер-ферма", {"Kora C9", "Plata M7", "DDR5 32 ГБ", "Pixel 3080", "БП 1000 Вт", "Большая башня"}, 6},
    {"Сервер-зверь", {"Kora X24 (серверный)", "Plata S7 (сервер)", "DDR4 64 ГБ", "БП 1000 Вт", "Жидкостное СО (AIO)"}, 5},
};
#define NBU ((int)(sizeof BU / sizeof BU[0]))

/* wc/wg — вклад CPU/GPU, ram/vram/sto — жёсткие требования, work — объём работы,
   rec — рекомендуемая производительность (по ней считается срок), sneed — нужная скорость диска */
typedef struct { const char *name; int wc, wg, ram, vram, sto, work, rec, reward, risk, sneed; } Tpl;
static const Tpl TP[] = {
    {"Расшифровка аудиозаписей", 100, 0, 4, 0, 50, 60, 15, 120, 5, 10},
    {"Сборка большого проекта", 100, 0, 8, 0, 100, 180, 40, 350, 8, 50},
    {"Рендер анимации", 40, 60, 16, 4, 200, 600, 80, 1100, 12, 50},
    {"Обучение нейросети", 20, 80, 32, 8, 500, 1800, 150, 3200, 20, 120},
    {"Анализ огромной базы данных", 70, 0, 64, 0, 1000, 2400, 120, 4200, 18, 200},
    {"Научная симуляция", 50, 50, 32, 12, 1000, 4000, 220, 7500, 25, 120},
    {"★ ЛЕГЕНДАРНЫЙ: рендер полнометражного фильма", 40, 60, 64, 16, 2000, 6000, 200, 60000, 15, 200},
};

typedef struct { int t, work, reward, days, due, risk; } Order;
typedef struct { int id, slot; } Item; /* slot -1 = на складе */
typedef struct {
    int magic, day, cash, nitems, deal;
    Item items[80];
    Order board[6], act;
    int has, won;
    double prog;
} Game;

#define NSLOT 11
static Game G;
static char line[256];
static char savepath[512];
static char msg[300];

static const int SK[NSLOT] = {K_CPU, K_MB, K_RAM, K_RAM, K_GPU, K_PSU, K_COOL, K_STO, K_STO, K_CASE, K_UPS};
static const char *SN[NSLOT] = {"Процессор", "Мат. плата", "ОЗУ 1", "ОЗУ 2", "Видеокарта", "Блок питания",
                                "Кулер", "Диск 1", "Диск 2", "Корпус", "ИБП"};
static const int SOPT[NSLOT] = {0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1}; /* необязательные слоты */

static double rnd(void) { return (double)rand() / RAND_MAX; }

/* ---------- вывод: рамки, колонки, полосы ---------- */

static int vlen(const char *s) {
    int n = 0;
    while (*s) {
        if (*s == '\033') {
            while (*s && *s != 'm') s++;
            if (*s) s++;
            continue;
        }
        if (((unsigned char)*s & 0xC0) != 0x80) n++;
        s++;
    }
    return n;
}

/* дописывает s в dst, обрезая/добирая пробелами до w видимых колонок; right=1 — выравнивание вправо */
static void appw(char *dst, size_t dn, const char *s, int w, int right) {
    size_t o = strlen(dst);
    int len = vlen(s), pad = w - len;
    if (right && pad > 0)
        while (pad-- > 0 && o + 1 < dn) dst[o++] = ' ';
    int vis = 0, trunc = 0;
    while (*s && o + 12 < dn) {
        if (*s == '\033') {
            while (*s) {
                dst[o++] = *s;
                if (*s++ == 'm') break;
            }
            continue;
        }
        if (vis >= w) { trunc = 1; break; }
        dst[o++] = *s++;
        while (*s && ((unsigned char)*s & 0xC0) == 0x80) dst[o++] = *s++;
        vis++;
    }
    if (trunc) { memcpy(dst + o, RST, 4); o += 4; }
    if (!right)
        while (vis < w && o + 1 < dn) { dst[o++] = ' '; vis++; }
    dst[o] = 0;
}

static void hr(const char *l, const char *r) {
    printf(BC "%s", l);
    for (int i = 0; i < W + 2; i++) printf("─");
    printf("%s" RST "\n", r);
}
static void titled(const char *l, const char *r, const char *title) {
    printf(BC "%s─ " RST BLD ACC "%s" RST BC " ", l, title);
    int n = W - 1 - vlen(title);
    for (int i = 0; i < n; i++) printf("─");
    printf("%s" RST "\n", r);
}
static void top(const char *t) { titled("╭", "╮", t); }
static void sep(const char *t) {
    if (t[0]) titled("├", "┤", t);
    else hr("├", "┤");
}
static void bottom(void) { hr("╰", "╯"); }

static void bl(const char *fmt, ...) {
    char raw[1024], out[1400];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(raw, sizeof raw, fmt, ap);
    va_end(ap);
    out[0] = 0;
    appw(out, sizeof out, raw, W, 0);
    printf(BC "│" RST " %s " BC "│" RST "\n", out);
}
static void blr(const char *l, const char *r) {
    char o[1500];
    o[0] = 0;
    int lw = W - vlen(r) - 1;
    appw(o, sizeof o, l, lw > 0 ? lw : 0, 0);
    strcat(o, " ");
    strcat(o, r);
    printf(BC "│" RST " %s " BC "│" RST "\n", o);
}

static void bar(char *d, size_t n, double f, int w, const char *col) {
    if (f < 0) f = 0;
    if (f > 1) f = 1;
    int k = (int)(f * w + 0.5);
    snprintf(d, n, "%s", col);
    for (int i = 0; i < k; i++) strncat(d, "█", n - strlen(d) - 1);
    strncat(d, DIM, n - strlen(d) - 1);
    for (int i = k; i < w; i++) strncat(d, "░", n - strlen(d) - 1);
    strncat(d, RST, n - strlen(d) - 1);
}

static void flash(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, sizeof msg, fmt, ap);
    va_end(ap);
}

static void ask(const char *p) {
    printf("%s", p);
    fflush(stdout);
    if (!fgets(line, sizeof line, stdin)) exit(0);
}
static void prompt(void) { ask("  " ACC BLD "▸ " RST); }
static void pause_(void) { ask("\n  " DIM "Enter ▸ " RST); }

/* ---------- данные ---------- */

static void savegame(void) {
    FILE *f = fopen(savepath, "wb");
    if (!f) return;
    fwrite(&G, sizeof G, 1, f);
    fclose(f);
}
static int pid(const char *n) {
    for (int i = 0; i < NCAT; i++)
        if (!strcmp(CAT[i].name, n)) return i;
    return 0;
}
static int price(int id) { return id == G.deal ? CAT[id].price * 80 / 100 : CAT[id].price; }
static int addItem(int id, int slot) {
    if (G.nitems >= 80) return -1;
    G.items[G.nitems].id = id;
    G.items[G.nitems].slot = slot;
    return G.nitems++;
}
static void removeItem(int i) {
    memmove(&G.items[i], &G.items[i + 1], (G.nitems - i - 1) * sizeof(Item));
    G.nitems--;
}
static int slotIdx(int s) {
    for (int i = 0; i < G.nitems; i++)
        if (G.items[i].slot == s) return i;
    return -1;
}
static const Part *slotPart(int s) {
    int i = slotIdx(s);
    return i < 0 ? NULL : &CAT[G.items[i].id];
}
static void newDeal(void) {
    int id;
    do id = rand() % NCAT; while (CAT[id].price < 30 || id == G.deal);
    G.deal = id;
}

static void spec(const Part *p, char *b, size_t n) {
    switch (p->kind) {
    case K_CPU: snprintf(b, n, "S%d · перф %d · %d Вт", p->a + 1, p->b, p->c); break;
    case K_MB: snprintf(b, n, "S%d · DDR%d · до %d ГБ", p->a + 1, p->b, p->c); break;
    case K_RAM: snprintf(b, n, "DDR%d · %d ГБ", p->a, p->b); break;
    case K_GPU: snprintf(b, n, "перф %d · %d ГБ · %d Вт", p->a, p->b, p->c); break;
    case K_PSU: snprintf(b, n, "%d Вт", p->a); break;
    case K_COOL: snprintf(b, n, "отводит до %d Вт", p->a); break;
    case K_CASE: snprintf(b, n, "продув: +%d Вт охлаждению", p->a); break;
    case K_UPS: snprintf(b, n, "гасит сбои питания на %d%%", p->a); break;
    default: snprintf(b, n, "%d ГБ · скор. %d · %s", p->a, p->b, p->c ? "SSD" : "HDD"); break;
    }
}

typedef struct {
    int ok, ram, vram, sto, sspeed, watts, weakPsu, weakCool, psuw, tdp, coolcap, ups;
    double cpu, gpu, ramf;
    char err[160];
} Stat;

static Stat calc(void) {
    Stat st;
    memset(&st, 0, sizeof st);
    st.ok = 1;
    st.ramf = 1.0;
#define ERR(m) do { if (st.ok) snprintf(st.err, sizeof st.err, "%s", m); st.ok = 0; } while (0)
    const Part *cpu = slotPart(0), *mb = slotPart(1), *gpu = slotPart(4), *psu = slotPart(5), *cool = slotPart(6),
               *cs = slotPart(9), *ups = slotPart(10);
    if (!cpu) ERR("нет процессора");
    if (!mb) ERR("нет материнской платы");
    if (cpu && mb && cpu->a != mb->a) ERR("сокет процессора не подходит к плате");
    for (int i = 2; i <= 3; i++) {
        const Part *r = slotPart(i);
        if (!r) continue;
        st.ram += r->b;
        if (mb && r->a != mb->b) ERR("тип памяти не подходит к плате");
        if (r->a == 5) st.ramf = 1.1;
    }
    if (!st.ram) ERR("нет оперативной памяти");
    if (mb && st.ram > mb->c) ERR("плата не тянет столько памяти");
    for (int i = 7; i <= 8; i++) {
        const Part *s = slotPart(i);
        if (!s) continue;
        st.sto += s->a;
        if (s->b > st.sspeed) st.sspeed = s->b;
    }
    if (!st.sto) ERR("нет накопителя");
    if (!psu) ERR("нет блока питания");
    if (!cool) ERR("нет кулера");
    st.cpu = cpu ? cpu->b : 0;
    st.gpu = gpu ? gpu->a : 3; /* встроенная графика */
    st.vram = gpu ? gpu->b : 0;
    st.tdp = cpu ? cpu->c : 0;
    st.watts = st.tdp + (gpu ? gpu->c : 0) + 60;
    st.ups = ups ? ups->a : 0;
    st.coolcap = (cool ? cool->a : 0) + (cs ? cs->a : 0);
    if (psu) {
        st.psuw = psu->a;
        if (psu->a < st.watts * 0.8) ERR("блок питания слабее нагрузки, ПК не включается");
        else if (psu->a < st.watts * 1.2) st.weakPsu = 1;
    }
    if (cool && cpu && st.coolcap < cpu->c) st.weakCool = 1;
    return st;
}

static double thr(const Stat *st, int t) {
    const Tpl *p = &TP[t];
    double base = (st->cpu * p->wc + st->gpu * p->wg) / 100.0;
    double sf = 0.6 + 0.4 * (st->sspeed >= p->sneed ? 1.0 : (double)st->sspeed / p->sneed);
    return base * st->ramf * sf * (st->weakCool ? 0.75 : 1.0);
}

/* 1 — требования выполнены; иначе пишет причину */
static int reqmet(const Stat *st, int t, char *why, size_t n) {
    const Tpl *p = &TP[t];
    why[0] = 0;
    if (st->ram < p->ram) snprintf(why + strlen(why), n - strlen(why), "ОЗУ %d ГБ; ", p->ram);
    if (st->vram < p->vram) snprintf(why + strlen(why), n - strlen(why), "видеопамять %d ГБ; ", p->vram);
    if (st->sto < p->sto) snprintf(why + strlen(why), n - strlen(why), "диск %d ГБ; ", p->sto);
    return why[0] == 0;
}

static Order gen(int slot) {
    Order o;
    int t = slot + (rand() % 2);
    if (t > 5) t = 5;
    if (slot == 5) t = 6;
    const Tpl *p = &TP[t];
    o.t = t;
    o.work = p->work * (85 + rand() % 31) / 100;
    o.reward = p->reward * (85 + rand() % 31) / 100;
    o.risk = p->risk + (rand() % 9) - 4;
    if (o.risk < 2) o.risk = 2;
    o.days = (o.work + p->rec - 1) / p->rec + 2;
    o.due = 0;
    return o;
}

/* ---------- интерфейс ---------- */

static const char *loadcol(double f, double warn, double bad) { return f < warn ? GOOD : f < bad ? WARN : BAD; }

static void header(void) {
    Stat st = calc();
    char a[800], b[400], c[400], d[400];
    printf(CLR);
    hr("╭", "╮");
    snprintf(a, sizeof a, BLD ACC "◆ VEYRA" RST BLD TXT " PCFORGE" RST DIM "  " VERSION);
    snprintf(b, sizeof b, DIM "День " TXT BLD "%d" RST DIM "  │  %s" BLD "$%d" RST, G.day, G.cash < 0 ? BAD : GOLD, G.cash);
    blr(a, b);
    hr("├", "┤");
    if (st.ok) {
        bl(GOOD "●" RST BLD " ОНЛАЙН" RST DIM "   CPU " TXT "%.0f" DIM " · GPU " TXT "%.0f" DIM " · ОЗУ " TXT "%d ГБ" DIM
                " · Диск " TXT "%d ГБ",
           st.cpu, st.gpu, st.ram, st.sto);
        bar(c, sizeof c, (double)st.watts / st.psuw, 10, loadcol((double)st.watts / st.psuw, 0.834, 1.25));
        bar(d, sizeof d, (double)st.tdp / st.coolcap, 10, loadcol((double)st.tdp / st.coolcap, 1.0001, 1.3));
        bl(DIM "Питание " RST "%s" TXT " %d/%d Вт" DIM "    Охлаждение " RST "%s" TXT " %d/%d Вт", c, st.watts, st.psuw, d,
           st.tdp, st.coolcap);
    } else {
        bl(BAD "●" RST BLD BAD " ОФФЛАЙН" RST DIM "  %s", st.err);
    }
    if (G.has) {
        bar(c, sizeof c, G.prog / G.act.work, 14, ACC);
        snprintf(a, sizeof a, DIM "Заказ " RST "%s" BLD TXT " %.0f%%" RST DIM "  " TXT "%s", c, 100.0 * G.prog / G.act.work,
                 TP[G.act.t].name);
        snprintf(b, sizeof b, DIM "срок: день " TXT "%d", G.act.due);
        blr(a, b);
    } else {
        bl(DIM "Заказ: нет активного. Загляни на доску заказов.");
    }
    bottom();
    if (msg[0]) {
        printf("  %s\n", msg);
        msg[0] = 0;
    }
}

static void banner(void) {
    static const char *R[6] = {"██╗   ██╗███████╗██╗   ██╗██████╗  █████╗ ", "██║   ██║██╔════╝╚██╗ ██╔╝██╔══██╗██╔══██╗",
                               "██║   ██║█████╗   ╚████╔╝ ██████╔╝███████║", "╚██╗ ██╔╝██╔══╝    ╚██╔╝  ██╔══██╗██╔══██║",
                               " ╚████╔╝ ███████╗   ██║   ██║  ██║██║  ██║", "  ╚═══╝  ╚══════╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═╝"};
    static const int GC[6] = {51, 45, 39, 69, 105, 141};
    printf(CLR "\n");
    for (int i = 0; i < 6; i++) printf("                  \033[38;5;%dm%s" RST "\n", GC[i], R[i]);
    printf("\n                             " C(141) "▒░ " RST BLD TXT "P C F O R G E" RST C(141) " ░▒" RST "\n");
    printf("                    " DIM "симулятор сборки ПК и аренды мощностей" RST "\n\n");
}

/* совместимость детали с текущей сборкой: 1 ок, 2 на пределе, 0 нет, -1 неприменимо */
static int compat(const Part *p) {
    const Part *cpu = slotPart(0), *mb = slotPart(1), *psu = slotPart(5), *cs = slotPart(9);
    Stat st = calc();
    switch (p->kind) {
    case K_CPU: return mb ? (mb->a == p->a) : -1;
    case K_MB: {
        const Part *r = slotPart(2);
        if (!cpu && !r) return -1;
        return (!cpu || cpu->a == p->a) && (!r || r->a == p->b);
    }
    case K_RAM: return mb ? (p->a == mb->b && p->b <= mb->c) : -1;
    case K_GPU: {
        if (!psu) return -1;
        double need = (cpu ? cpu->c : 0) + p->c + 60;
        return psu->a >= need * 1.2 ? 1 : psu->a >= need * 0.8 ? 2 : 0;
    }
    case K_PSU: return p->a >= st.watts * 1.2 ? 1 : p->a >= st.watts * 0.8 ? 2 : 0;
    case K_COOL: return cpu ? (p->a + (cs ? cs->a : 0) >= cpu->c) : -1;
    default: return -1;
    }
}
static const char *mark(int c) { return c == 1 ? GOOD "✔" RST : c == 2 ? WARN "~" RST : c == 0 ? BAD "✖" RST : DIM "·" RST; }

static int cmpPrice(const void *a, const void *b) { return CAT[*(const int *)a].price - CAT[*(const int *)b].price; }

static int buyPart(int id, int q) {
    int done = 0;
    for (int i = 0; i < q; i++) {
        if (G.cash < price(id) || G.nitems >= 80) break;
        G.cash -= price(id);
        addItem(id, -1);
        done++;
    }
    return done;
}

static void shopCat(int k) {
    for (;;) {
        int map[NCAT], n = 0;
        for (int i = 0; i < NCAT; i++)
            if (CAT[i].kind == k) map[n++] = i;
        qsort(map, n, sizeof(int), cmpPrice);
        header();
        top(KN[k]);
        char row[900], t[200];
        row[0] = 0;
        appw(row, sizeof row, DIM "№", 3, 0);
        appw(row, sizeof row, DIM "Название", 26, 0);
        appw(row, sizeof row, DIM "Характеристики", 29, 0);
        appw(row, sizeof row, DIM "Сов.", 4, 0);
        appw(row, sizeof row, DIM "Цена", 8, 1);
        bl("%s", row);
        sep("ТОВАРЫ");
        for (int j = 0; j < n; j++) {
            const Part *p = &CAT[map[j]];
            char sp[100], nm[100], pr[40];
            spec(p, sp, sizeof sp);
            row[0] = 0;
            snprintf(t, sizeof t, ACC BLD "%d" RST, j + 1);
            appw(row, sizeof row, t, 3, 0);
            if (map[j] == G.deal) snprintf(nm, sizeof nm, GOLD "★ " TXT "%s", p->name);
            else snprintf(nm, sizeof nm, TXT "%s", p->name);
            appw(row, sizeof row, nm, 26, 0);
            snprintf(t, sizeof t, DIM "%s", sp);
            appw(row, sizeof row, t, 29, 0);
            snprintf(t, sizeof t, " %s  ", mark(compat(p)));
            appw(row, sizeof row, t, 4, 0);
            snprintf(pr, sizeof pr, "%s$%d" RST, map[j] == G.deal ? GOLD : G.cash >= price(map[j]) ? GOOD : BAD, price(map[j]));
            appw(row, sizeof row, pr, 8, 1);
            bl("%s", row);
        }
        bottom();
        printf("  " DIM "Сов.: " RST "%s подходит к сборке  %s на пределе  %s не подойдёт     " DIM "★ скидка дня" RST "\n", mark(1),
               mark(2), mark(0));
        printf("  " DIM "Номер — купить · «3 x2» — купить две · 0 — назад" RST "\n");
        prompt();
        int num = 0, q = 1;
        int r = sscanf(line, " %d x%d", &num, &q);
        if (r < 1 || num == 0) return;
        if (num < 1 || num > n) continue;
        if (r < 2 || q < 1) q = 1;
        if (q > 20) q = 20;
        int id = map[num - 1];
        int done = buyPart(id, q);
        if (done == 0)
            flash(BAD "Не вышло: не хватает денег или склад полон." RST);
        else
            flash(GOOD "Куплено: " TXT "%s" GOOD " ×%d. " DIM "Деталь на складе — установи в «Мой ПК».", CAT[id].name, done);
    }
}

static int install(int n);

static void shopBundles(void) {
    for (;;) {
        header();
        top("ГОТОВЫЕ НАБОРЫ · скидка 10%");
        int tot[NBU];
        for (int i = 0; i < NBU; i++) {
            int sum = 0;
            for (int j = 0; j < BU[i].n; j++) sum += CAT[pid(BU[i].items[j])].price;
            tot[i] = sum * 9 / 10;
            char l[300], r[100];
            snprintf(l, sizeof l, ACC BLD "%d" RST "  " TXT BLD "%s" RST, i + 1, BU[i].name);
            snprintf(r, sizeof r, DIM "вместо $%d  %s" BLD "$%d" RST, sum, G.cash >= tot[i] ? GOOD : BAD, tot[i]);
            blr(l, r);
            char cur[400];
            cur[0] = 0;
            for (int j = 0; j < BU[i].n; j++) {
                if (cur[0] && vlen(cur) + vlen(BU[i].items[j]) + 3 > 66) {
                    bl("   " DIM "%s", cur);
                    cur[0] = 0;
                }
                if (cur[0]) strcat(cur, " · ");
                strcat(cur, BU[i].items[j]);
            }
            if (cur[0]) bl("   " DIM "%s", cur);
            if (i < NBU - 1) bl("");
        }
        bottom();
        printf("  " DIM "Номер — купить набор (старые детали уйдут на склад) · 0 — назад" RST "\n");
        prompt();
        int num = atoi(line);
        if (num < 1 || num > NBU) return;
        if (G.cash < tot[num - 1] || G.nitems + BU[num - 1].n > 80) {
            flash(BAD "Не хватает денег или места на складе." RST);
            continue;
        }
        G.cash -= tot[num - 1];
        for (int j = 0; j < BU[num - 1].n; j++) {
            int idx = addItem(pid(BU[num - 1].items[j]), -1);
            if (idx >= 0) install(idx);
        }
        flash(GOOD "Набор «%s» куплен и установлен." RST DIM " Проверь сборку: слабые места видно в шапке.", BU[num - 1].name);
        return;
    }
}

static void shop(void) {
    for (;;) {
        header();
        top("МАГАЗИН ▸ VEYRA PARTS");
        const Part *d = &CAT[G.deal];
        char sp[100], l[300], r[200];
        spec(d, sp, sizeof sp);
        bl(GOLD BLD "★ ПРЕДЛОЖЕНИЕ ДНЯ" RST DIM "   скидка 20%%, обновляется каждые 3 дня");
        snprintf(l, sizeof l, "  " TXT BLD "%s" RST DIM "  %s", d->name, sp);
        snprintf(r, sizeof r, "%s" BLD "$%d" RST DIM " вместо $%d  " ACC BLD "[d]" RST, G.cash >= price(G.deal) ? GOOD : BAD,
                 price(G.deal), d->price);
        blr(l, r);
        sep("КАТЕГОРИИ");
        for (int k = 0; k < NK; k++) {
            int cnt = 0, mn = 1 << 30;
            for (int i = 0; i < NCAT; i++)
                if (CAT[i].kind == k) {
                    cnt++;
                    if (CAT[i].price < mn) mn = CAT[i].price;
                }
            snprintf(l, sizeof l, " " ACC BLD "%2d" RST "  " TXT "%s", k + 1, KN[k]);
            snprintf(r, sizeof r, DIM "%d шт. · от $%d", cnt, mn);
            blr(l, r);
        }
        snprintf(l, sizeof l, " " ACC BLD "%2d" RST "  " GOLD "Готовые наборы", NK + 1);
        snprintf(r, sizeof r, DIM "%d набора · −10%%", NBU);
        blr(l, r);
        blr(" " ACC BLD " 0" RST "  " DIM "Назад", "");
        bottom();
        prompt();
        if (line[0] == 'd' || line[0] == 'D') {
            int done = buyPart(G.deal, 1);
            if (done) flash(GOOD "Куплено по скидке: " TXT "%s" RST, CAT[G.deal].name);
            else flash(BAD "Не хватает денег или склад полон." RST);
            continue;
        }
        int k = atoi(line);
        if (k == 0) return;
        if (k >= 1 && k <= NK) shopCat(k - 1);
        else if (k == NK + 1) shopBundles();
    }
}

/* ставит деталь в слот (если занят — старая уходит на склад) */
static int install(int n) {
    if (n < 0 || n >= G.nitems || G.items[n].slot != -1) return 0;
    int kind = CAT[G.items[n].id].kind, target = -1;
    for (int s = 0; s < NSLOT; s++)
        if (SK[s] == kind && slotIdx(s) < 0) { target = s; break; }
    if (target < 0)
        for (int s = 0; s < NSLOT; s++)
            if (SK[s] == kind) { target = s; break; }
    int old = slotIdx(target);
    if (old >= 0) G.items[old].slot = -1;
    G.items[n].slot = target;
    return 1;
}
/* ставит только в свободный слот */
static int installFree(int n) {
    if (n < 0 || n >= G.nitems || G.items[n].slot != -1) return 0;
    int kind = CAT[G.items[n].id].kind;
    for (int s = 0; s < NSLOT; s++)
        if (SK[s] == kind && slotIdx(s) < 0) { G.items[n].slot = s; return 1; }
    return 0;
}

static void mypc(void) {
    for (;;) {
        header();
        top("МОЙ ПК ▸ СЛОТЫ");
        for (int s = 0; s < NSLOT; s++) {
            const Part *p = slotPart(s);
            char row[900], t[200], b[120] = "";
            row[0] = 0;
            snprintf(t, sizeof t, ACC "[%d]", s);
            appw(row, sizeof row, t, 5, 0);
            snprintf(t, sizeof t, DIM "%s", SN[s]);
            appw(row, sizeof row, t, 14, 0);
            if (p) {
                spec(p, b, sizeof b);
                snprintf(t, sizeof t, TXT BLD "%s", p->name);
                appw(row, sizeof row, t, 25, 0);
                snprintf(t, sizeof t, DIM "%s", b);
                appw(row, sizeof row, t, 28, 0);
            } else {
                appw(row, sizeof row, SOPT[s] ? DIM "— пусто (необязательно)" : WARN "— пусто —", 53, 0);
            }
            bl("%s", row);
        }
        sep("СКЛАД");
        int any = 0;
        for (int i = 0; i < G.nitems; i++)
            if (G.items[i].slot == -1) {
                char row[900], t[200], b[120];
                const Part *p = &CAT[G.items[i].id];
                spec(p, b, sizeof b);
                row[0] = 0;
                snprintf(t, sizeof t, ACC "#%d", i);
                appw(row, sizeof row, t, 5, 0);
                snprintf(t, sizeof t, TXT "%s", p->name);
                appw(row, sizeof row, t, 27, 0);
                snprintf(t, sizeof t, DIM "%s", b);
                appw(row, sizeof row, t, 28, 0);
                snprintf(t, sizeof t, DIM "$%d", p->price * 6 / 10);
                appw(row, sizeof row, t, 8, 1);
                bl("%s", row);
                any = 1;
            }
        if (!any) bl(DIM "пусто");
        sep("КОМАНДЫ");
        bl(ACC "i N" RST DIM " поставить со склада   " ACC "u S" RST DIM " снять из слота   " ACC "s N" RST DIM " продать (60%%)");
        bl(ACC "a" RST DIM "   поставить всё в свободные слоты        " ACC "0" RST DIM "   назад");
        bottom();
        prompt();
        char c;
        int n;
        int r = sscanf(line, " %c %d", &c, &n);
        if (r == 2) {
            if (c == 'i') {
                if (!install(n)) flash(BAD "Нет такой детали на складе." RST);
            } else if (c == 'u') {
                int i = slotIdx(n);
                if (i >= 0) G.items[i].slot = -1;
            } else if (c == 's') {
                if (n >= 0 && n < G.nitems && G.items[n].slot == -1) {
                    G.cash += CAT[G.items[n].id].price * 6 / 10;
                    flash(GOOD "Продано: %s" RST, CAT[G.items[n].id].name);
                    removeItem(n);
                }
            }
        } else if (r == 1 && c == 'a') {
            int k = 0;
            for (int i = 0; i < G.nitems; i++) k += installFree(i);
            flash(k ? GOOD "Установлено деталей: %d" RST : DIM "Свободных подходящих слотов нет." RST, k);
        } else if (line[0] == '0' || line[0] == 'q') return;
    }
}

static void orders(void) {
    for (;;) {
        header();
        Stat st = calc();
        top("ДОСКА ЗАКАЗОВ");
        int first = 1;
        for (int i = 0; i < 6; i++) {
            if (G.board[i].t < 0) {
                if (i == 5 && !G.won) {
                    if (!first) sep("");
                    bl(GOLD "[6] ★ закрытый контракт" RST DIM " — появится на 8-й день");
                    first = 0;
                }
                continue;
            }
            const Order *o = &G.board[i];
            const Tpl *p = &TP[o->t];
            char why[160], l[400], r[200], c1[40], bb[200];
            if (!first) sep("");
            first = 0;
            snprintf(l, sizeof l, "%s" BLD "[%d]" RST " %s%s", o->t == 6 ? GOLD : ACC, i + 1, o->t == 6 ? GOLD : TXT, p->name);
            snprintf(r, sizeof r, GOLD BLD "$%d" RST, o->reward);
            blr(l, r);
            int k = (o->risk + 4) / 5;
            if (k > 5) k = 5;
            const char *rc = o->risk < 10 ? GOOD : o->risk < 18 ? WARN : BAD;
            snprintf(bb, sizeof bb, "%s", rc);
            for (int j = 0; j < 5; j++) strcat(bb, j < k ? "■" : "□");
            strcat(bb, RST);
            bl("    " DIM "срок " TXT "%d дн." DIM " · риск %s " DIM "%d%% · заказчик не заплатит: " TXT "%d%%", o->days, bb, o->risk,
               o->risk > 10 ? (o->risk - 10) / 2 : 0);
            snprintf(l, sizeof l, "    " DIM "нужно: ОЗУ %s%d ГБ" DIM " · видео %s%d ГБ" DIM " · диск %s%d ГБ", st.ram >= p->ram ? TXT : BAD,
                     p->ram, st.vram >= p->vram ? TXT : BAD, p->vram, st.sto >= p->sto ? TXT : BAD, p->sto);
            if (!st.ok) snprintf(r, sizeof r, BAD "ПК оффлайн");
            else if (!reqmet(&st, o->t, why, sizeof why)) snprintf(r, sizeof r, BAD "НЕ ХВАТАЕТ");
            else {
                double t = thr(&st, o->t);
                int est = (int)((o->work + t - 1) / t);
                snprintf(c1, sizeof c1, "%s", est <= o->days ? GOOD : WARN);
                snprintf(r, sizeof r, "%s" BLD "ГОТОВ" RST DIM " · ~%d дн.", c1, est);
            }
            blr(l, r);
        }
        sep("КОМАНДЫ");
        bl(ACC "a N" RST DIM " взять заказ    " ACC "c" RST DIM " отменить текущий (штраф 10%%)    " ACC "0" RST DIM " назад");
        bottom();
        prompt();
        char c;
        int n = 0;
        int r = sscanf(line, " %c %d", &c, &n);
        if (r >= 1 && c == 'c') {
            if (G.has) {
                G.cash -= G.act.reward / 10;
                G.has = 0;
                flash(WARN "Заказ отменён, штраф $%d." RST, G.act.reward / 10);
            }
        } else if (r == 2 && c == 'a') {
            n--;
            char why[160];
            if (n < 0 || n > 5 || G.board[n].t < 0) continue;
            if (G.has) flash(BAD "Сначала закончи текущий заказ." RST);
            else if (!st.ok) flash(BAD "ПК не запускается: %s" RST, st.err);
            else if (!reqmet(&st, G.board[n].t, why, sizeof why)) flash(BAD "Не хватает: %s" RST, why);
            else {
                G.act = G.board[n];
                G.act.due = G.day + G.act.days;
                G.has = 1;
                G.prog = 0;
                G.board[n].t = -1;
                flash(GOOD "Заказ принят. Срок — день %d." RST, G.act.due);
            }
        } else if (line[0] == '0' || line[0] == 'q') return;
    }
}

static void burn(void) {
    int cand[4] = {0, 1, 4, 5}, have[4], n = 0;
    for (int i = 0; i < 4; i++)
        if (slotIdx(cand[i]) >= 0) have[n++] = cand[i];
    if (!n) return;
    int s = have[rand() % n];
    int i = slotIdx(s);
    bl("   " BAD BLD "■ ПЕРЕГРУЗКА!" RST BAD " Сгорело: %s (слот «%s»). Деталь потеряна.", CAT[G.items[i].id].name, SN[s]);
    removeItem(i);
}

/* один день (печатает строки в открытую рамку); 1 — если случилось что-то важное */
static int dayStep(void) {
    int ev = 0;
    G.day++;
    for (int i = 0; i < 5; i++)
        if (rnd() < 0.12) G.board[i] = gen(i);
    if (G.day % 3 == 0) newDeal();
    if (G.day >= 8 && G.board[5].t < 0 && !G.won && !(G.has && G.act.t == 6)) {
        G.board[5] = gen(5);
        bl(GOLD "★ День %d: открылся легендарный контракт!" RST, G.day);
        ev = 1;
    }
    if (!G.has) {
        bl(DIM "День %d  простой", G.day);
        return ev;
    }
    Stat st = calc();
    int dead = 0;
    if (!st.ok) {
        bl(DIM "День %d  " BAD "ПК не запускается (%s), работа стоит.", G.day, st.err);
        ev = 1;
    } else {
        double t = thr(&st, G.act.t);
        int cost = st.watts * 25 / 1000;
        if (cost < 1) cost = 1;
        G.cash -= cost;
        G.prog += t;
        bl(DIM "День %d  " RST "+%.0f работы · электричество " BAD "−$%d" RST, G.day, t, cost);
        double p = 0.02 + G.act.risk * 0.002 + (st.weakPsu ? 0.06 : 0) + (st.weakCool ? 0.05 : 0);
        if (rnd() < p) {
            ev = 1;
            double r = rnd();
            if (r < 0.45) {
                double loss = (st.weakPsu ? 0.30 : 0.15) * (100 - st.ups) / 100.0;
                G.prog *= 1 - loss;
                if (st.ups >= 40) bl("   " WARN "▲ Сбой питания: ИБП смягчил удар, потеряно %.0f%% прогресса.", loss * 100);
                else bl("   " WARN "▲ Сбой питания: потеряно %.0f%% прогресса.", loss * 100);
            } else if (r < 0.75) {
                G.prog -= t;
                if (G.prog < 0) G.prog = 0;
                bl("   " WARN "▲ Баг в софте заказчика: день работы пропал.");
            } else if (st.weakCool || st.weakPsu) {
                burn();
                dead = 1;
            } else {
                G.act.work += G.act.work / 10;
                bl("   " WARN "▲ Заказчик прислал новые данные: объём работы +10%%.");
            }
        }
    }
    if (!dead && G.has && G.prog >= G.act.work) {
        int pay = G.act.reward, late = G.day - G.act.due;
        if (late > 0) {
            int cut = late * 20 > 60 ? 60 : late * 20;
            pay = pay * (100 - cut) / 100;
            bl("   " WARN "▲ Сдано с опозданием на %d дн.: оплата −%d%%.", late, cut);
        }
        int scam = G.act.risk > 10 ? (G.act.risk - 10) / 2 : 0;
        if (rand() % 100 < scam) {
            pay = 0;
            bl("   " BAD BLD "■ ЗАКАЗЧИК ИСЧЕЗ." RST BAD " Работа сделана, денег нет.");
        } else {
            bl("   " GOOD BLD "● ЗАКАЗ ВЫПОЛНЕН!" RST GOOD " Получено $%d.", pay);
        }
        G.cash += pay;
        if (G.act.t == 6 && pay > 0) {
            G.won = 1;
            bl("");
            bl(GOLD BLD "   ★★★  ПОБЕДА  ★★★");
            bl(GOLD "   Твой компьютер вытянул легендарный контракт. Играй дальше, если хочешь.");
        }
        G.has = 0;
        ev = 1;
    } else if (!dead && G.has && G.day > G.act.due + 3) {
        int pen = G.act.reward / 5;
        G.cash -= pen;
        bl("   " BAD "■ Заказ провален: срок вышел. Штраф $%d.", pen);
        G.has = 0;
        ev = 1;
    }
    return ev;
}

static void newGame(void) {
    memset(&G, 0, sizeof G);
    G.magic = MAGIC;
    G.day = 1;
    G.cash = 1500;
    addItem(pid("Kora C1 (старый)"), 0);
    addItem(pid("Plata B1"), 1);
    addItem(pid("DDR4 4 ГБ"), 2);
    addItem(pid("HDD 500 ГБ"), 7);
    addItem(pid("БП 300 Вт"), 5);
    addItem(pid("Боксовый кулер"), 6);
    for (int i = 0; i < 5; i++) G.board[i] = gen(i);
    G.board[5].t = -1;
    G.act.t = -1;
    G.deal = -1;
    newDeal();
    banner();
    top("ДОБРО ПОЖАЛОВАТЬ");
    bl("Ты начинаешь с древним компьютером и " GOLD "$1500" RST ".");
    bl("");
    bl(BLD "Цель:" RST " собрать мощный ПК и выполнить " GOLD "ЛЕГЕНДАРНЫЙ контракт" RST " ($60000).");
    bl(DIM "Он откроется на 8-й день и потребует много ОЗУ, видеопамяти и места на диске.");
    sep("КАК ИГРАТЬ");
    bl(ACC "▸" RST " бери заказы на доске — они приносят деньги;");
    bl(ACC "▸" RST " покупай детали в магазине, ставь их в разделе «Мой ПК»;");
    bl(ACC "▸" RST " сокет процессора и тип памяти должны совпадать с платой;");
    bl(ACC "▸" RST " БП и кулер должны тянуть нагрузку — иначе сбои и сгоревшие детали;");
    bl(ACC "▸" RST " ИБП и корпус с продувом снижают риски;");
    bl(ACC "▸" RST " рискованные заказы платят больше, но заказчик может исчезнуть.");
    bottom();
    pause_();
}

int main(void) {
    srand((unsigned)time(NULL));
    const char *home = getenv("HOME");
    snprintf(savepath, sizeof savepath, "%s/.veyra-pcforge.sav", home ? home : ".");
    int loaded = 0;
    FILE *f = fopen(savepath, "rb");
    if (f) {
        Game tmp;
        if (fread(&tmp, sizeof tmp, 1, f) == 1 && tmp.magic == MAGIC) {
            banner();
            ask("  Найдено сохранение. " ACC "Enter" RST " — продолжить, " ACC "n" RST " — новая игра: ");
            if (line[0] != 'n' && line[0] != 'N') { G = tmp; loaded = 1; }
        }
        fclose(f);
    }
    if (!loaded) newGame();
    for (;;) {
        if (G.cash < -300) {
            header();
            top("КОНЕЦ");
            bl(BAD BLD "БАНКРОТСТВО." RST BAD " Долги слишком велики. Игра окончена.");
            bottom();
            remove(savepath);
            return 0;
        }
        header();
        top("ГЛАВНОЕ МЕНЮ");
        static const char *M[6][2] = {{"Магазин", "детали, наборы, скидка дня"},
                                      {"Мой ПК", "сборка, склад, продажа"},
                                      {"Доска заказов", "контракты и риски"},
                                      {"Прожить день", "время идёт, заказ считается"},
                                      {"До события", "до 7 дней, пока что-то не случится"},
                                      {"Сохранить", "прогресс на диск"}};
        for (int i = 0; i < 6; i++) {
            char l[200], r[120];
            snprintf(l, sizeof l, " " ACC BLD "%d" RST "  " TXT "%s", i + 1, M[i][0]);
            snprintf(r, sizeof r, DIM "%s", M[i][1]);
            blr(l, r);
        }
        blr(" " ACC BLD "0" RST "  " DIM "Выход (с автосохранением)", "");
        bottom();
        prompt();
        switch (atoi(line)) {
        case 1: shop(); break;
        case 2: mypc(); break;
        case 3: orders(); break;
        case 4:
            header();
            top("ХРОНИКА");
            dayStep();
            bottom();
            pause_();
            break;
        case 5:
            header();
            top("ХРОНИКА");
            for (int i = 0; i < 7; i++)
                if (dayStep() || G.cash < -300) break;
            bottom();
            pause_();
            break;
        case 6:
            savegame();
            flash(GOOD "Сохранено: %s" RST, savepath);
            break;
        case 0:
            savegame();
            printf("\n  " DIM "Сохранено. До встречи в VEYRA Pcforge!" RST "\n\n");
            return 0;
        }
    }
}
