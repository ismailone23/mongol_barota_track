import cv2
import mediapipe as mp
import time
from data_media.gestures import GestureRecognizer

BaseOptions = mp.tasks.BaseOptions
HandLandmarker = mp.tasks.vision.HandLandmarker
HandLandmarkerOptions = mp.tasks.vision.HandLandmarkerOptions
VisionRunningMode = mp.tasks.vision.RunningMode


class HandDetector:

    def __init__(self, model_path, num_hands=1):

        options = HandLandmarkerOptions(
            base_options=BaseOptions(
                model_asset_path=model_path
            ),
            running_mode=VisionRunningMode.VIDEO,
            num_hands=num_hands
        )

        self.landmarker = (
            HandLandmarker.create_from_options(options)
        )

        self.timestamp_ms = int(time.time() * 1000)

    def process(self, frame):

        rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

        mp_image = mp.Image(
            image_format=mp.ImageFormat.SRGB,
            data=rgb
        )

        result = self.landmarker.detect_for_video(
            mp_image,
            self.timestamp_ms
        )

        

        self.timestamp_ms += 1

        hands = []

        if result.hand_landmarks:

            for idx, lm in enumerate(result.hand_landmarks):

                label = (
                    result.handedness[idx][0]
                    .category_name
                )

                fingers = GestureRecognizer.get_fingers_up(lm, label)

                gesture = GestureRecognizer.get_gesture(fingers)

                hands.append({
                    "gesture": gesture,
                    "fingers": fingers,
                    "landmarks": lm,
                    "hand_label": label
                })

        return hands

    def close(self):
        self.landmarker.close()