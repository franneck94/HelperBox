import matplotlib.pyplot as plt
import numpy as np


def Sign(p1, p2, p3):
    return (p1[0] - p3[0]) * (p2[1] - p3[1]) - (p2[0] - p3[0]) * (p1[1] - p3[1])


def PointInGameRectangle(pt, v1, v2, v3, v4):
    return PointInTriangle(pt, v1, v2, v3) or PointInTriangle(pt, v4, v2, v3)


def PointInTriangle(pt, v1, v2, v3):
    b1 = Sign(pt, v1, v2) < 0.0
    b2 = Sign(pt, v2, v3) < 0.0
    b3 = Sign(pt, v3, v1) < 0.0

    return (b1 == b2) and (b2 == b3)


def get_rect(p1: list, p2: list, width: float, offset: float):
    dist = np.linalg.norm(np.array(p1) - np.array(p2))
    d_t = dist + offset
    t = d_t / dist
    p1_x = (1.0 - t) * p2[0] + t * p1[0]
    p1_y = (1.0 - t) * p2[1] + t * p1[1]

    p2_x = (1.0 - t) * p1[0] + t * p2[0]
    p2_y = (1.0 - t) * p1[1] + t * p2[1]

    adj_p1 = [p1_x, p1_y]
    adj_p2 = [p2_x, p2_y]

    hald_width = width * 0.5
    delta_x = adj_p1[0] - adj_p2[0]
    delta_y = adj_p1[1] - adj_p2[1]
    dist = np.linalg.norm(np.array(adj_p1) - np.array(adj_p2))

    v1 = [
        adj_p1[0] + ((-delta_y) / dist) * hald_width,
        adj_p1[1] + (delta_x / dist) * hald_width,
    ]
    v2 = [
        adj_p1[0] + ((-delta_y) / dist) * (-hald_width),
        adj_p1[1] + (delta_x / dist) * (-hald_width),
    ]
    v3 = [
        adj_p2[0] - (delta_y / dist) * hald_width,
        adj_p2[1] + (delta_x / dist) * hald_width,
    ]
    v4 = [
        adj_p2[0] + ((-delta_y) / dist) * (-hald_width),
        adj_p2[1] + (delta_x / dist) * (-hald_width),
    ]
    return v1, v2, v3, v4


def main() -> None:
    aggro_range = 1280
    p1 = [float(np.random.uniform(low=-10_000, high=10_000)) for _ in range(2)]
    p2 = [float(np.random.uniform(low=-10_000, high=10_000)) for _ in range(2)]
    width = 1400

    v1, v2, v3, v4 = get_rect(p1, p2, width, aggro_range)

    plt.scatter(*p1, c="lightblue")
    plt.scatter(*p2, c="orange")
    plt.plot([v1[0], v2[0]], [v1[1], v2[1]], c="blue")
    plt.plot([v1[0], v3[0]], [v1[1], v3[1]], c="blue")
    plt.plot([v4[0], v2[0]], [v4[1], v2[1]], c="blue")
    plt.plot([v4[0], v3[0]], [v4[1], v3[1]], c="blue")

    ps = np.random.uniform(low=min(*p1, *p2) - 100,
                           high=max(*p1, *p2) + 100, size=(500, 2))
    for p in ps:
        if PointInGameRectangle(p, v1, v2, v3, v4):
            plt.scatter(*p, c="green", s=20)
        else:
            plt.scatter(*p, c="red", s=20)

    plt.show()


if __name__ == "__main__":
    main()
