from dataclasses import dataclass

import numpy as np
import matplotlib.pyplot as plt


@dataclass
class GamePos:
    x: float
    y: float
    z: int


def RotatePoint(theta: float, point: GamePos) -> GamePos:
    m = np.array([
        [np.cos(theta), -np.sin(theta)],
        [np.sin(theta), np.cos(theta)]
    ])
    p = np.array([point.x, point.y])
    r = np.dot(m, p)
    return GamePos(r[0], r[1], 0)


def GetLineBasedOnGamePosAndAngle(point: GamePos, theta: float) -> tuple:
    # orth_point = GamePos(point.y, -point.x, 0)
    orth_point = RotatePoint(theta + (np. pi / 2.0), point)
    point2 = GamePos(point.x + orth_point.x, point.x + orth_point.y, 0)

    slope = (point2.y - point.y) / (point2.x - point.x)
    bias = point.y - slope * point.x
    return slope, bias


def GamePosIsBelowLine(slope: float, bias: float, point: GamePos) -> bool:
    y_l = slope * point.x + bias
    y_p = point.y

    y_delta = y_l - y_p

    if y_delta > 0.0 and slope > 0.0:
        return True
    if y_delta > 0.0 and slope < 0.0:
        return True
    if y_delta < 0.0 and slope > 0.0:
        return False
    if y_delta < 0.0 and slope < 0.0:
        return False

    return False


def plot_line(point: GamePos, m : float, b: float, color: str) -> None:
    ly1 = m * (point.x - 2.0) + b
    ly2 = m * (point.x + 2.0) + b
    l_p1 = GamePos(0.0, ly1, 0)
    l_p2 = GamePos(4.0, ly2, 0)
    plt.plot([l_p1.x, l_p2.x], [l_p1.y, l_p2.y], color=color)


def main() -> None:
    theta = np.pi / 18.0
    player_pos = GamePos(2.0, 2.0, 0)
    agent_pos = GamePos(1.0, 1.0, 0)
    m1, b1 = GetLineBasedOnGamePosAndAngle(player_pos, 0.0)
    m2, b2 = GetLineBasedOnGamePosAndAngle(player_pos, theta)
    plt.scatter(player_pos.x, player_pos.y, c="lightblue")
    plt.scatter(agent_pos.x, agent_pos.y, c="orange")
    plot_line(player_pos, m1, b1, "blue")
    plot_line(player_pos, m2, b2, "red")
    plt.show()


if __name__ == "__main__":
    main()
