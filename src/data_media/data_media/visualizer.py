import cv2


class Visualizer:

    @staticmethod
    def draw(frame, hands):

        for hand in hands:

            lm = hand["landmarks"]

            h, w, _ = frame.shape

            for point in lm:

                x = int(point.x * w)
                y = int(point.y * h)

                cv2.circle(
                    frame,
                    (x, y),
                    5,
                    (0, 255, 0),
                    -1
                )

            cv2.putText(
                frame,
                hand["gesture"],
                (20, 50),
                cv2.FONT_HERSHEY_SIMPLEX,
                1,
                (0, 255, 0),
                2
            )

        return frame