import numpy as np

import gpuencode

def main():
    cuda_context = gpuencode.CudaContext(0)
    with cuda_context:
        session_config = gpuencode.SessionConfig()
        session_config.codec = gpuencode.Codec.HEVC
        session_config.preset = gpuencode.Preset.P1
        session_config.tuning = gpuencode.Tuning.HighQuality
        session_config.format = gpuencode.Format.ABGR
        session_config.width = 640
        session_config.height = 480
        session_config.framerateNum = 20

        encoder = gpuencode.Encoder(session_config)
        encoder.video_filepath = 'video.hevc'
        with encoder as e:
            for i in range(200):
                x = np.sin(np.linspace(0, 2 * session_config.width / session_config.height * np.pi, session_config.width) + i * np.pi / 200)
                y = np.sin(np.linspace(0, 2 * np.pi, session_config.height).reshape(1, -1).T - 10 * i * np.pi / 200)
                a = np.dstack(np.meshgrid(x, y))
                a = np.dstack((a, np.zeros(a.shape)))

                a[..., 0] += 1
                a[..., 1] += 1
                a[..., 0] *= 0.5 * 255.
                a[..., 1] *= 0.5 * 255.
                frame = np.round(a).astype(np.uint8)
                frame[..., 3] = 255
                e(i, frame)
                print('.', end='', flush=True)
            print('')
    print('done')

if __name__ == '__main__':
    main()