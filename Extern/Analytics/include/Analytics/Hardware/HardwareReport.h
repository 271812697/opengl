#pragma once

namespace Analytics::Hardware
{

	struct HardwareReport final
	{
		/* CPU load for every process on the machine (%) */
		float CPULoad;

		/* GPU load for every process on the machine (%) */
		float GPULoad;

		/* Used RAM on the machine for every process (MB) */
		float RAMUsed;

		/* Free RAM on the machine (MB) */
		float RAMFree;

		/* Maximum RAM available (MB) */
		float RAMMax;
	};
}