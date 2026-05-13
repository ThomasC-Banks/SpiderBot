#pragma once

void face_init(void);

void face_normal(void);
void face_normal_pupil(void);
void face_happy(void);
void face_blink(void);
void face_look_left(void);
void face_look_right(void);
void face_sleepy(void);
void face_excited(void);
void face_wink(void);

/* Animation fluide : fermeture + ouverture des yeux */
void face_animate_blink(void);