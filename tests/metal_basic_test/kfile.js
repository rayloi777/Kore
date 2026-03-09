const project = new Project('metal_basic_test');

await project.addProject(findKore());

project.addFile('sources/**');
project.addKongDir('shaders');
project.setDebugDir('deployment');

project.addIncludeDir('../../build/Kong-osx-metal');

project.flatten();

resolve(project);
