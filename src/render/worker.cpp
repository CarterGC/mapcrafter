/*
 * Copyright 2012, 2013 Moritz Hilscher
 *
 * This file is part of mapcrafter.
 *
 * mapcrafter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mapcrafter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mapcrafter.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "worker.h"

namespace mapcrafter {
namespace render {

Worker::Worker() {

}

Worker::~Worker() {

}

void Worker::setWorld(std::shared_ptr<mc::WorldCache> world,
		std::shared_ptr<TileSet> tileset,
		std::shared_ptr<BlockImages> blockimages) {
	this->world = world;
	this->tileset = tileset;
	this->blockimages = blockimages;
}

void Worker::setWork(const config2::MapcrafterConfigFile& config,
		const config2::MapSection& map_config,
		const std::set<TilePath>& tiles, const std::set<TilePath>& tiles_skip) {
	this->config = config;
	this->map_config = map_config;
	this->tiles = tiles;
	this->tiles_skip = tiles_skip;
}

void Worker::renderRecursive(const TilePath& tile, Image& image) {
	// if this is tile is not required or we should skip it, load it from file
	if (!tileset->isTileRequired(tile) || tiles_skip.count(tile)) {
		fs::path file = config.getOutputPath(tile.toString() + ".png");
		if (!image.readPNG(file.string())) {
			std::cerr << "Unable to read tile " << tile.toString() << " from " << file << std::endl;
			std::cerr << tileset->isTileRequired(tile) << " " << tiles_skip.count(tile) << std::endl;
		}
	} else if (tile.getDepth() == tileset->getDepth()) {
		// this tile is a render tile, render it
		renderer.renderTile(tile.getTilePos(), image);

		/*
		int size = settings.tile_size;
		for (int x = 0; x < size; x++)
			for (int y = 0; y < size; y++) {
				if (x < 5 || x > size-5)
					tile.setPixel(x, y, rgba(0, 0, 255, 255));
				if (y < 5 || y > size-5)
					tile.setPixel(x, y, rgba(0, 0, 255, 255));
			}
		*/

		// save it
		//saveTile(settings.output_dir, path, tile);

		// update progress
		/*
		settings.progress++;
		if (settings.show_progress) {
			settings.progress_bar.update(settings.progress);
		}
		*/
	} else {
		// this tile is a composite tile, we need to compose it from its children
		// just check, if children 1, 2, 3, 4 exists, render it, resize it to the half size
		// and blit it to the properly position
		int size = map_config.getTextureSize() * 64;
		image.setSize(size, size);

		Image other;
		Image resized;
		if (tileset->hasTile(tile + 1)) {
			renderRecursive(tile + 1, other);
			other.resizeHalf(resized);
			image.simpleblit(resized, 0, 0);
			other.clear();
		}
		if (tileset->hasTile(tile + 2)) {
			renderRecursive(tile + 2, other);
			other.resizeHalf(resized);
			image.simpleblit(resized, size / 2, 0);
			other.clear();
		}
		if (tileset->hasTile(tile + 3)) {
			renderRecursive(tile + 3, other);
			other.resizeHalf(resized);
			image.simpleblit(resized, 0, size / 2);
			other.clear();
		}
		if (tileset->hasTile(tile + 4)) {
			renderRecursive(tile + 4, other);
			other.resizeHalf(resized);
			image.simpleblit(resized, size / 2, size / 2);
		}

		/*
		for (int x = 0; x < size; x++)
			for (int y = 0; y < size; y++) {
				if (x < 5 || x > size-5)
					tile.setPixel(x, y, rgba(255, 0, 0, 255));
				if (y < 5 || y > size-5)
					tile.setPixel(x, y, rgba(255, 0, 0, 255));
			}
		*/

		// then save tile
		//saveTile(settings.output_dir, path, tile);
	}
}

void Worker::operator()() {
	renderer = TileRenderer(world, blockimages, map_config);
}

} /* namespace render */
} /* namespace mapcrafter */
