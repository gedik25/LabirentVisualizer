# 🎯 Labirent Geliştirme Planı

**Tarih:** 2026-02-02  
**Durum:** Onay Bekliyor

---

## 📋 Özet

Bu plan, Labirent projesindeki UI/UX sorunlarını ve eksik entegrasyonları düzeltmek için hazırlanmıştır.

---

## 🔧 Yapılacak Değişiklikler

### Faz 1: UI Panel ve Pencere Düzeltmeleri
**Dosyalar:** `main.cpp`, `UIPanel.cpp`, `UIPanel.hpp`

| # | Görev | Detay | Dosya |
|---|-------|-------|-------|
| 1.1 | Pencere boyutunu büyüt | 1200x800 → 1600x1000 | `main.cpp` |
| 1.2 | STATUS panel genişlet | 180px → 220px | `UIPanel.cpp` |
| 1.3 | Generator bilgisini göster | Panel'e "Gen: Prim" satırı ekle | `UIPanel.cpp/hpp` |
| 1.4 | Difficulty bilgisini göster | Panel'e "Diff: Medium" satırı ekle | `UIPanel.cpp/hpp` |
| 1.5 | Terrain durumunu göster | Panel'e "Terrain: ON/OFF" satırı ekle | `UIPanel.cpp/hpp` |

**Tahmini Değişiklik:** ~50 satır

---

### Faz 2: Menu Entegrasyonu
**Dosyalar:** `main.cpp`

| # | Görev | Detay |
|---|-------|-------|
| 2.1 | `menu.onApply` callback'i bağla | Menüden seçilen ayarları uygula |
| 2.2 | `menu.onGeneratorChange` bağla | Generator tipini güncelle |
| 2.3 | `menu.onSolverChange` bağla | Solver tipini güncelle |
| 2.4 | `menu.onDifficultyChange` bağla | Zorluk seviyesini güncelle |
| 2.5 | `menu.onSizeChange` bağla | Grid boyutunu güncelle |

**Tahmini Değişiklik:** ~40 satır

---

### Faz 3: Terrain Bilgi Paneli
**Dosyalar:** `UIPanel.cpp`, `UIPanel.hpp`, `main.cpp`

| # | Görev | Detay |
|---|-------|-------|
| 3.1 | Terrain banner ekle | Terrain açıkken üstte bilgi banner'ı |
| 3.2 | Path cost göster | A*/Dijkstra için toplam yol maliyeti |
| 3.3 | Maliyet karşılaştırma | BFS path cost vs A* path cost göster |

**Tahmini Değişiklik:** ~60 satır

---

### Faz 4: Comparison Mode Entegrasyonu
**Dosyalar:** `main.cpp`, `ComparisonView.cpp`, `MenuSystem.cpp`

| # | Görev | Detay |
|---|-------|-------|
| 4.1 | X tuşu ile comparison mode aç | Split screen görünümü başlat |
| 4.2 | Menüye comparison bölümü ekle | Algoritma seçimi için UI |
| 4.3 | 2'li ve 4'lü split screen | Sayı tuşları ile seçim (2 veya 4) |
| 4.4 | Sonuç tablosu | Her algoritmanın istatistiklerini göster |

**Tahmini Değişiklik:** ~100 satır

---

## 📁 Etkilenen Dosyalar

```
src/
├── main.cpp                    [MAJOR] - Pencere, menu callbacks, comparison
├── Visualization/
│   ├── UIPanel.hpp            [MINOR] - Yeni setter fonksiyonları
│   └── UIPanel.cpp            [MAJOR] - Panel genişliği, yeni satırlar
├── UI/
│   ├── MenuSystem.cpp         [MINOR] - Comparison mode bölümü
│   └── ComparisonView.cpp     [MINOR] - Split screen render
└── Core/
    └── (değişiklik yok)
```

---

## 🎨 UI Değişiklikleri (Görsel)

### Mevcut STATUS Panel:
```
┌─────────────────────┐
│ STATUS              │
│ Grid:   25 x 25     │
│ State:  Ready       │
│ Anim:   ON          │
│ Delay:  10 ms       │
│ FPS:    60          │
└─────────────────────┘
```

### Yeni STATUS Panel:
```
┌─────────────────────────┐
│ STATUS                  │
│ Grid:      25 x 25      │
│ State:     Ready        │
│ Generator: Recursive    │  ← YENİ
│ Difficulty: Medium      │  ← YENİ
│ Terrain:   OFF          │  ← YENİ
│ Anim:      ON           │
│ Delay:     10 ms        │
│ FPS:       60           │
└─────────────────────────┘
```

### Terrain Banner (Terrain ON iken):
```
┌──────────────────────────────────────────────────────────┐
│ 🌍 TERRAIN MODE: Weighted Pathfinding Active             │
│    A*/Dijkstra algorithms use terrain costs              │
└──────────────────────────────────────────────────────────┘
```

### Comparison Mode (Split Screen):
```
┌─────────────────────────────────────────────────────────────────┐
│              ALGORITHM COMPARISON (2 Algorithms)                │
├───────────────────────────┬─────────────────────────────────────┤
│           BFS             │              A*                     │
│    ┌─────────────────┐    │    ┌─────────────────┐              │
│    │                 │    │    │                 │              │
│    │   [Labirent]    │    │    │   [Labirent]    │              │
│    │                 │    │    │                 │              │
│    └─────────────────┘    │    └─────────────────┘              │
│    Visited: 245           │    Visited: 187                     │
│    Path: 42               │    Path: 38                         │
│    Cost: 42.0             │    Cost: 35.2  ✓ (Best)             │
│    Time: 12ms             │    Time: 15ms                       │
└───────────────────────────┴─────────────────────────────────────┘
```

---

## ⌨️ Yeni Klavye Kısayolları

| Tuş | Eylem | Not |
|-----|-------|-----|
| X | Comparison mode aç/kapat | Split screen |
| 2 | (Comparison modda) 2 algoritma | - |
| 4 | (Comparison modda) 4 algoritma | - |

---

## 📊 Uygulama Sırası

```
Faz 1 (UI Panel) ──► Faz 2 (Menu) ──► Faz 3 (Terrain) ──► Faz 4 (Comparison)
     │                    │                 │                    │
     ▼                    ▼                 ▼                    ▼
  Test Et             Test Et           Test Et              Test Et
```

---

## ✅ Başarı Kriterleri

- [ ] Pencere 1600x1000 boyutunda açılıyor
- [ ] STATUS panel'de Generator, Difficulty, Terrain bilgisi görünüyor
- [ ] Menu'den "Apply & Close" yapınca ayarlar uygulanıyor
- [ ] Shift+1-5 basınca ekranda generator değişimi görünüyor
- [ ] Terrain açıkken banner görünüyor
- [ ] A*/Dijkstra çalışırken path cost görünüyor
- [ ] X tuşu comparison mode açıyor
- [ ] Split screen'de 2 algoritma yan yana çalışıyor

---

## 🚀 Başlangıç

Plan onaylandıktan sonra Faz 1'den başlanacak.

**Tahmini Toplam Süre:** 30-45 dakika
**Toplam Değişiklik:** ~250 satır kod
