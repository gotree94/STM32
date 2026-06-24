using System.Text.Json.Serialization;

namespace SlamViewer;

public record class GroundTruth(
    [property: JsonPropertyName("x")] double X,
    [property: JsonPropertyName("y")] double Y,
    [property: JsonPropertyName("theta")] double Theta
);

public record class Odometry(
    [property: JsonPropertyName("left_encoder_ticks")] int LeftEncoderTicks,
    [property: JsonPropertyName("right_encoder_ticks")] int RightEncoderTicks,
    [property: JsonPropertyName("distance_cm")] double DistanceCm
);

public record class ImuData(
    [property: JsonPropertyName("accel")] AccelData? Accel,
    [property: JsonPropertyName("gyro")] GyroData? Gyro
);

public record class AccelData(
    [property: JsonPropertyName("x")] double X,
    [property: JsonPropertyName("y")] double Y,
    [property: JsonPropertyName("z")] double Z
);

public record class GyroData(
    [property: JsonPropertyName("x")] double X,
    [property: JsonPropertyName("y")] double Y,
    [property: JsonPropertyName("z")] double Z
);

public record class ScanReading(
    [property: JsonPropertyName("angle_relative")] int AngleRelative,
    [property: JsonPropertyName("angle_world")] double AngleWorld,
    [property: JsonPropertyName("distance")] double Distance,
    [property: JsonPropertyName("true_distance")] double TrueDistance,
    [property: JsonPropertyName("sensor_id")] int SensorId
);

public record class FrameData(
    [property: JsonPropertyName("timestamp")] double Timestamp,
    [property: JsonPropertyName("ground_truth")] GroundTruth? GroundTruth,
    [property: JsonPropertyName("odometry")] Odometry? Odometry,
    [property: JsonPropertyName("imu")] ImuData? Imu,
    [property: JsonPropertyName("scan")] List<ScanReading>? Scan,
    [property: JsonPropertyName("is_turn")] bool IsTurn
);

public record class RobotData(
    [property: JsonPropertyName("wheel_radius_cm")] double WheelRadiusCm,
    [property: JsonPropertyName("wheel_base_cm")] double WheelBaseCm,
    [property: JsonPropertyName("encoder_cpr")] int EncoderCpr,
    [property: JsonPropertyName("speed_cm_s")] double SpeedCmS,
    [property: JsonPropertyName("wall_offset_cm")] double WallOffsetCm,
    [property: JsonPropertyName("scan_interval_cm")] double ScanIntervalCm,
    [property: JsonPropertyName("scan_angle_step_deg")] int ScanAngleStepDeg,
    [property: JsonPropertyName("noise_sigma")] double NoiseSigma
);

public record class EnvironmentData(
    [property: JsonPropertyName("width")] double Width,
    [property: JsonPropertyName("height")] double Height,
    [property: JsonPropertyName("obstacles")] List<List<double>>? Obstacles,
    [property: JsonPropertyName("circles")] List<List<double>>? Circles
);

public record class SimulationData(
    [property: JsonPropertyName("environment")] EnvironmentData? Environment,
    [property: JsonPropertyName("robot")] RobotData? Robot,
    [property: JsonPropertyName("data")] List<FrameData>? Data
);
