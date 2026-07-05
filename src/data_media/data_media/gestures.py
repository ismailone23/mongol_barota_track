class GestureRecognizer:

    @staticmethod
    def get_fingers_up(lm, hand_label):

        fingers = {
            "Thumb": False,
            "Index": False,
            "Middle": False,
            "Ring": False,
            "Pinky": False,
        }

        fingers["Thumb"] = abs(lm[4].x - lm[3].x) > 0.02
        fingers["Index"] = lm[8].y < lm[6].y
        fingers["Middle"] = lm[12].y < lm[10].y
        fingers["Ring"] = lm[16].y < lm[14].y
        fingers["Pinky"] = lm[20].y < lm[18].y

        return fingers

    @staticmethod
    def get_gesture(fingers):

        thumb = fingers["Thumb"]
        index = fingers["Index"]
        middle = fingers["Middle"]
        ring = fingers["Ring"]
        pinky = fingers["Pinky"]

        if not any([thumb, index, middle, ring, pinky]):
            return "Fist"

        if all([thumb, index, middle, ring, pinky]):
            return "Open Hand"

        if thumb and not any([index, middle, ring, pinky]):
            return "Thumb"

        if index and not any([thumb, middle, ring, pinky]):
            return "Point"

        if (
            index and middle
            and not thumb
            and not ring
            and not pinky
        ):
            return "Peace"

        if (
            index and middle and ring
            and not thumb
            and not pinky
        ):
            return "Three"

        if (
            index and middle and ring and pinky
            and not thumb
        ):
            return "Four"

        return "Unknown"